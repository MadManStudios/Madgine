#include "../directx12lib.h"

#include "directx12queryheap.h"

#include "../directx12rendercontext.h"

namespace Engine {
namespace Render {

    static constexpr size_t sHeapSize = 32;

    DirectX12QueryHeap::DirectX12QueryHeap(D3D12_QUERY_HEAP_TYPE type)
        : mType(type)
    {
        D3D12_QUERY_HEAP_DESC heapDesc = {};
        heapDesc.Count = sHeapSize;
        heapDesc.NodeMask = 0;
        heapDesc.Type = type;
        GetDevice()->CreateQueryHeap(&heapDesc, IID_PPV_ARGS(&mHeap));
    }

    DirectX12QueryHeap::~DirectX12QueryHeap()
    {
    }

    DirectX12QueryHeap &DirectX12QueryHeap::operator=(DirectX12QueryHeap &&other)
    {
        std::swap(mHeap, other.mHeap);
        std::swap(mIndex, other.mIndex);
        std::swap(mType, other.mType);
        return *this;
    }

    size_t DirectX12QueryHeap::allocate(size_t count)
    {
        if (mIndex + count > sHeapSize)
            mIndex = 0;
        size_t result = mIndex;
        mIndex = (mIndex + count) % sHeapSize;
        return result;
    }

    void DirectX12QueryHeap::deallocate(size_t handle)
    {        
    }

    ID3D12QueryHeap *DirectX12QueryHeap::resource() const
    {
        return mHeap;
    }
}
}