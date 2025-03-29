#pragma once

namespace Engine {
namespace Render {

    struct MADGINE_DIRECTX12_EXPORT DirectX12DescriptorHeap
    {
        DirectX12DescriptorHeap() = default;
        DirectX12DescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE type);
        ~DirectX12DescriptorHeap();

        DirectX12DescriptorHeap &operator=(DirectX12DescriptorHeap &&other);

        OffsetPtr allocate(size_t count = 1);
        void deallocate(OffsetPtr handle);
        void deallocate(D3D12_GPU_DESCRIPTOR_HANDLE handle);

        D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle(OffsetPtr index);
        D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle(D3D12_GPU_DESCRIPTOR_HANDLE handle);
        D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle(OffsetPtr index);
        OffsetPtr fromGpuHandle(D3D12_GPU_DESCRIPTOR_HANDLE handle);

        void addShaderResourceView(OffsetPtr index, ID3D12Resource *resource, const D3D12_SHADER_RESOURCE_VIEW_DESC *desc);

        ID3D12DescriptorHeap *resource() const;

    private:
        D3D12_DESCRIPTOR_HEAP_TYPE mType;
        ReleasePtr<ID3D12DescriptorHeap> mHeap;
        ReleasePtr<ID3D12DescriptorHeap> mUploadHeap;
        size_t mIndex = 0;
        std::vector<size_t> mFreeList;
    };

}
}