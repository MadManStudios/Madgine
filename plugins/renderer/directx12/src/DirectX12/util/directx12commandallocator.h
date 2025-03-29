#pragma once

#include "directx12commandlist.h"

#include "Madgine/render/future.h"

#include "Modules/debug/profiler/profilerthread.h"

#if ENABLE_TASK_TRACKING
#    include "Modules/debug/tasktracking/tasktracker.h"
#endif

namespace Engine {
namespace Render {

    struct DirectX12CommandAllocator {
        DirectX12CommandAllocator(D3D12_COMMAND_LIST_TYPE type, std::string_view name, DirectX12DescriptorHeap *descriptorHeap, DirectX12QueryHeap *timestampHeap);
        ~DirectX12CommandAllocator();

        void setup();

        DirectX12CommandList fetchCommandList();
        RenderFuture ExecuteCommandList(ReleasePtr<ID3D12GraphicsCommandList> list, ReleasePtr<ID3D12CommandAllocator> allocator, std::vector<Any> discardResources);

        RenderFuture currentFence();

        bool isComplete(RenderFuture fut);

        RenderFuture signalFence();
        void wait(RenderFuture fut);

        void waitForIdle();

        ID3D12CommandQueue *queue();

#if ENABLE_TASK_TRACKING
        Debug::Tasks::TaskTracker mTracker;
#endif

    private:
        D3D12_COMMAND_LIST_TYPE mType;
        std::string mName;
        DirectX12DescriptorHeap *mDescriptorHeap;
        DirectX12QueryHeap *mTimestampHeap;
        ReleasePtr<ID3D12CommandQueue> mCommandQueue;
        std::vector<std::tuple<RenderFuture, ReleasePtr<ID3D12CommandAllocator>, std::vector<Any>>> mAllocatorPool;
        std::vector<ReleasePtr<ID3D12GraphicsCommandList>> mCommandListPool;

        uint64_t mLastCompletedFenceValue = 0;
        uint64_t mNextFenceValue;
        ID3D12Fence *mFence;

#if ENABLE_PROFILER
        Debug::Profiler::ProfilerThread mProfiler;
        Debug::Profiler::ProcessStats mStats;
        struct PendingStats {
            size_t mIndex;
        };
        PendingStats mPendingStats;
#endif

#if ENABLE_PROFILER || ENABLE_TASK_TRACKING
        double mGPUFrequency;
        UINT64 mGPUTimestampOffset;
#endif
    };

}
}