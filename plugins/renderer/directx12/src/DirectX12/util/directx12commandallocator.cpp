#include "../directx12lib.h"

#include "../directx12rendercontext.h"
#include "directx12commandallocator.h"

#include "Modules/debug/profiler/profile.h"
#include "Modules/debug/profiler/profiler.h"

namespace Engine {
namespace Render {

    double getCPUFrequency()
    {
        static double freq = []() {
            LARGE_INTEGER buffer;
            bool result = QueryPerformanceFrequency(&buffer);
            assert(result);
            return buffer.QuadPart / 1000000000.0;
        }();
        return freq;
    }

    DirectX12CommandAllocator::DirectX12CommandAllocator(D3D12_COMMAND_LIST_TYPE type, std::string_view name, DirectX12DescriptorHeap *descriptorHeap, DirectX12QueryHeap *timestampHeap)
        : mType(type)
        , mName(name)
        , mDescriptorHeap(descriptorHeap)
        , mTimestampHeap(timestampHeap)
#if ENABLE_PROFILER
        , mProfiler(mName.c_str())
        , mStats("Execution")
#endif
    {
        Debug::Profiler::Profiler::getCurrent().registerThread(&mProfiler);
    }

    DirectX12CommandAllocator::~DirectX12CommandAllocator()
    {
        Debug::Profiler::Profiler::getCurrent().unregisterThread(&mProfiler);
    }

    void DirectX12CommandAllocator::setup()
    {
        D3D12_COMMAND_QUEUE_DESC queueDesc {};
        queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        queueDesc.Type = mType;

        HRESULT hr = GetDevice()->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&mCommandQueue));
        DX12_CHECK(hr);

#if ENABLE_PROFILER || ENABLE_TASK_TRACKING
        if (mType != D3D12_COMMAND_LIST_TYPE_COPY) {
            UINT64 timestampFrequency;
            hr = mCommandQueue->GetTimestampFrequency(&timestampFrequency);
            DX12_CHECK(hr);

            mGPUFrequency = timestampFrequency / 1000000000.0f;

            UINT64 gpuTimestamp, cpuTimestamp;
            hr = mCommandQueue->GetClockCalibration(&gpuTimestamp, &cpuTimestamp);
            DX12_CHECK(hr);
            mGPUTimestampOffset = gpuTimestamp - (mGPUFrequency / getCPUFrequency()) * cpuTimestamp;
        }
#endif

#if ENABLE_TASK_TRACKING
        mTracker.onAssign(this, Debug::StackTrace<1>::getCurrent(0));
#endif

        mLastCompletedFenceValue = 5;
        hr = GetDevice()->CreateFence(mLastCompletedFenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&mFence));
        DX12_CHECK(hr);

        mNextFenceValue = mLastCompletedFenceValue + 1;
    }

    DirectX12CommandList DirectX12CommandAllocator::fetchCommandList()
    {
        ReleasePtr<ID3D12CommandAllocator> alloc;

        if (!mAllocatorPool.empty()) {
            auto &[fenceValue, allocator, discard] = mAllocatorPool.front();

            if (isComplete(fenceValue)) {
                alloc = std::move(allocator);
                mAllocatorPool.erase(mAllocatorPool.begin());
                HRESULT hr = alloc->Reset();
                DX12_CHECK(hr);
            }
        }

        if (!alloc) {
            HRESULT hr = GetDevice()->CreateCommandAllocator(mType, IID_PPV_ARGS(&alloc));
            DX12_CHECK(hr);
        }

        ReleasePtr<ID3D12GraphicsCommandList> list;

        if (!mCommandListPool.empty()) {
            list = std::move(mCommandListPool.back());
            mCommandListPool.pop_back();
            HRESULT hr = list->Reset(alloc, nullptr);
            DX12_CHECK(hr);
        } else {
            HRESULT hr = GetDevice()->CreateCommandList(0, mType, alloc, nullptr, IID_PPV_ARGS(&list));
            DX12_CHECK(hr);
        }

        list->SetName(StringUtil::toWString(mName).c_str());

        if (mType == D3D12_COMMAND_LIST_TYPE_DIRECT) {
            ID3D12DescriptorHeap *heap = mDescriptorHeap->resource();
            list->SetDescriptorHeaps(1, &heap);
        }

#if ENABLE_PROFILER
        if (mType != D3D12_COMMAND_LIST_TYPE_COPY) {
            size_t index = mTimestampHeap->allocate(2);
            list->EndQuery(mTimestampHeap->resource(), D3D12_QUERY_TYPE_TIMESTAMP, index);
            mPendingStats.mIndex = index;
        }
#endif

        return { this, std::move(list), std::move(alloc) };
    }

    RenderFuture DirectX12CommandAllocator::ExecuteCommandList(ReleasePtr<ID3D12GraphicsCommandList> list, ReleasePtr<ID3D12CommandAllocator> allocator, std::vector<Any> discardResources)
    {
#if ENABLE_PROFILER
        if (mType != D3D12_COMMAND_LIST_TYPE_COPY) {
            list->EndQuery(mTimestampHeap->resource(), D3D12_QUERY_TYPE_TIMESTAMP, mPendingStats.mIndex + 1);
            struct ProfileHelper {
                ProfileHelper(std::pair<uint64_t, uint64_t> *data, DirectX12CommandAllocator *allocator)
                    : mData(data)
                    , mAllocator(allocator)
                {
                }
                ~ProfileHelper()
                {
                    uint64_t diff = mData->second - mData->first;
                    std::chrono::nanoseconds time { static_cast<uint64_t>(diff / mAllocator->mGPUFrequency) };
                    mAllocator->mProfiler.mStats.updateChild(&mAllocator->mStats, mAllocator->mStats.inject(time));

#    if ENABLE_TASK_TRACKING
                    std::chrono::high_resolution_clock::time_point startTimePoint {
                        std::chrono::nanoseconds {
                            static_cast<long long>((mData->first - mAllocator->mGPUTimestampOffset) / mAllocator->mGPUFrequency) }
                    };
                    std::chrono::high_resolution_clock::time_point endTimePoint {
                        std::chrono::nanoseconds {
                            static_cast<long long>((mData->second - mAllocator->mGPUTimestampOffset) / mAllocator->mGPUFrequency) }
                    };
                    mAllocator->mTracker.onEnter(mAllocator, startTimePoint);
                    mAllocator->mTracker.onReturn(mAllocator, endTimePoint);
#    endif
                }
                std::pair<uint64_t, uint64_t> *mData;
                DirectX12CommandAllocator *mAllocator;
            };
            void *address = DirectX12RenderContext::getSingleton().mReadbackAllocator.allocate(sizeof(std::pair<uint64_t, uint64_t>)).mAddress;
            auto [res, offset] = DirectX12RenderContext::getSingleton().mReadbackMemoryHeap.resolve(address);
            list->ResolveQueryData(mTimestampHeap->resource(), D3D12_QUERY_TYPE_TIMESTAMP, mPendingStats.mIndex, 2, res, offset);

            discardResources.emplace_back(Any::inplace<ProfileHelper>, static_cast<std::pair<uint64_t, uint64_t> *>(address), this);
        }
#endif

        HRESULT hr = list->Close();
        DX12_CHECK(hr);

        ID3D12CommandList *cList = list;
        mCommandQueue->ExecuteCommandLists(1, &cList);
        DX12_CHECK();

        if (allocator || !discardResources.empty())
            mAllocatorPool.emplace_back(mNextFenceValue, std::move(allocator), std::move(discardResources));

        mCommandListPool.push_back(std::move(list));

        return signalFence();
    }

    RenderFuture DirectX12CommandAllocator::currentFence()
    {
        return mNextFenceValue - 1;
    }

    bool DirectX12CommandAllocator::isComplete(RenderFuture fut)
    {
        if (!fut)
            return true;
        if (fut.value() > mLastCompletedFenceValue)
            mLastCompletedFenceValue = std::max(mLastCompletedFenceValue, mFence->GetCompletedValue());
        return fut.value() <= mLastCompletedFenceValue;
    }

    RenderFuture DirectX12CommandAllocator::signalFence()
    {
        mCommandQueue->Signal(mFence, mNextFenceValue);
        return mNextFenceValue++;
    }

    void DirectX12CommandAllocator::wait(RenderFuture fut)
    {
        if (fut) {
            mCommandQueue->Wait(mFence, fut.value());
        }
    }

    void DirectX12CommandAllocator::waitForIdle()
    {
        while (!isComplete(currentFence()))
            ;

        for (auto &[_1, _2, deleter] : mAllocatorPool) {
            deleter.clear();
        }
    }

    ID3D12CommandQueue *DirectX12CommandAllocator::queue()
    {
        return mCommandQueue;
    }

}
}