#pragma once

namespace Engine {
namespace Render {

    struct MADGINE_DIRECTX12_EXPORT DirectX12QueryHeap {
        DirectX12QueryHeap() = default;
        DirectX12QueryHeap(D3D12_QUERY_HEAP_TYPE type);
        ~DirectX12QueryHeap();

        DirectX12QueryHeap &operator=(DirectX12QueryHeap &&other);

        size_t allocate(size_t count = 1);
        void deallocate(size_t handle);        

        ID3D12QueryHeap *resource() const;

    private:
        D3D12_QUERY_HEAP_TYPE mType;
        ReleasePtr<ID3D12QueryHeap> mHeap;
        size_t mIndex = 0;
    };

}
}