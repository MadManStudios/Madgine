#pragma once

#include "Generic/allocator/concepts.h"

namespace Engine {
namespace Render {

    struct DirectX12HeapAllocator {

        static constexpr size_t goodSize = 8 * 1024 * 1024; //8MB

        DirectX12HeapAllocator(DirectX12DescriptorHeap &descHeap);

        void setup(size_t count);
        void reset();

        Block allocate(size_t size, size_t alignment = 1);
        void deallocate(Block block);

        ID3D12Heap *heap(size_t index);
        std::pair<ID3D12Resource *, size_t> resolve(void *ptr);

        D3D12_GPU_DESCRIPTOR_HANDLE descriptorTable() const;

    private:
        struct Heap {
            ReleasePtr<ID3D12Heap> mHeap;
            ReleasePtr<ID3D12Resource> mReservedResource;
        };
        std::vector<Heap> mHeaps;
        OffsetPtr mDescriptors;
        DirectX12DescriptorHeap &mDescriptorHeap;
    };

    struct DirectX12MappedHeapAllocator {

        static constexpr size_t goodSize = 8 * 1024 * 1024; //128MB

        DirectX12MappedHeapAllocator(D3D12_HEAP_TYPE type);

        void reset();

        Block allocate(size_t size, size_t alignment = 1);
        void deallocate(Block block);

        std::pair<ID3D12Resource *, size_t> resolve(void *ptr);

    private:
        D3D12_HEAP_TYPE mType;
        struct Page {
            ReleasePtr<ID3D12Resource> mResource;
            void *mMappedAddress;
        };
        std::vector<Page> mPages;
    };

}
}