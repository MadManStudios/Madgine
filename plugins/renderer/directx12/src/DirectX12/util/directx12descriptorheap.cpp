#include "../directx12lib.h"

#include "directx12descriptorheap.h"

#include "../directx12rendercontext.h"

namespace Engine {
namespace Render {

    DirectX12DescriptorHeap::DirectX12DescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE type)
        : mType(type)
    {
        D3D12_DESCRIPTOR_HEAP_DESC desc;
        ZeroMemory(&desc, sizeof(D3D12_DESCRIPTOR_HEAP_DESC));
        desc.Type = type;
        desc.NodeMask = 0;
        desc.NumDescriptors = 1000;
        desc.Flags = type == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

        HRESULT hr = GetDevice()->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&mHeap));
        DX12_CHECK(hr);


        ZeroMemory(&desc, sizeof(D3D12_DESCRIPTOR_HEAP_DESC));
        desc.Type = type;
        desc.NodeMask = 0;
        desc.NumDescriptors = 1;
        desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

        hr = GetDevice()->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&mUploadHeap));
        DX12_CHECK(hr);
    }

    DirectX12DescriptorHeap::~DirectX12DescriptorHeap()
    {
    }

    DirectX12DescriptorHeap &DirectX12DescriptorHeap::operator=(DirectX12DescriptorHeap &&other)
    {
        std::swap(mHeap, other.mHeap);
        std::swap(mUploadHeap, other.mUploadHeap);
        std::swap(mIndex, other.mIndex);
        std::swap(mType, other.mType);
        return *this;
    }

    OffsetPtr DirectX12DescriptorHeap::allocate(size_t count)
    {
        size_t index;
        if (count > 1 || mFreeList.empty()) {
            index = mIndex;
            mIndex += count;
            assert(mIndex <= 1000);
        } else {
            index = mFreeList.back();
            mFreeList.pop_back();
        }

        return OffsetPtr { index };
    }

    void DirectX12DescriptorHeap::deallocate(OffsetPtr handle)
    {
        mFreeList.push_back(handle.offset());
    }

    void DirectX12DescriptorHeap::deallocate(D3D12_GPU_DESCRIPTOR_HANDLE handle)
    {
        deallocate(fromGpuHandle(handle));
    }

    D3D12_CPU_DESCRIPTOR_HANDLE DirectX12DescriptorHeap::cpuHandle(OffsetPtr index)
    {
        UINT handleSize = GetDevice()->GetDescriptorHandleIncrementSize(mType);
        return mHeap->GetCPUDescriptorHandleForHeapStart() + index * handleSize;
    }

    D3D12_CPU_DESCRIPTOR_HANDLE DirectX12DescriptorHeap::cpuHandle(D3D12_GPU_DESCRIPTOR_HANDLE handle)
    {
        return cpuHandle(fromGpuHandle(handle));
    }

    D3D12_GPU_DESCRIPTOR_HANDLE DirectX12DescriptorHeap::gpuHandle(OffsetPtr index)
    {
        UINT handleSize = GetDevice()->GetDescriptorHandleIncrementSize(mType);
        return mHeap->GetGPUDescriptorHandleForHeapStart() + index * handleSize;
    }

    OffsetPtr DirectX12DescriptorHeap::fromGpuHandle(D3D12_GPU_DESCRIPTOR_HANDLE handle)
    {
        UINT handleSize = GetDevice()->GetDescriptorHandleIncrementSize(mType);
        return (handle - mHeap->GetGPUDescriptorHandleForHeapStart()) / handleSize;
    }

    void DirectX12DescriptorHeap::addShaderResourceView(OffsetPtr index, ID3D12Resource *resource, const D3D12_SHADER_RESOURCE_VIEW_DESC *desc)
    {
        GetDevice()->CreateShaderResourceView(resource, desc, mUploadHeap->GetCPUDescriptorHandleForHeapStart());
        GetDevice()->CopyDescriptorsSimple(1, cpuHandle(index), mUploadHeap->GetCPUDescriptorHandleForHeapStart(), mType);
    }

    ID3D12DescriptorHeap *DirectX12DescriptorHeap::resource() const
    {
        return mHeap;
    }
}
}