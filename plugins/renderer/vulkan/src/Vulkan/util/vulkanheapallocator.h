#pragma once

#include "Generic/allocator/concepts.h"

namespace Engine {
namespace Render {

    struct VulkanHeapAllocator {

        static constexpr size_t goodSize = 32 * 1024 * 1024; //8MB

        VulkanHeapAllocator(std::string_view name);

        void setup(size_t count);

        Block allocate(size_t size, size_t alignment = 1);
        void deallocate(Block block);

        /* ID3D12Heap *heap(size_t index);*/
        std::pair<VkBuffer, size_t> resolve(void *ptr);

        /* D3D12_GPU_DESCRIPTOR_HANDLE descriptorTable() const;*/

    private:
        struct Heap {
            VulkanPtr<VkBuffer, &vkDestroyBuffer> mBuffer;
            VulkanPtr<VkDeviceMemory, &vkFreeMemory> mMemory;
        };
        std::vector<Heap> mHeaps;
        /* OffsetPtr mDescriptors;
        DirectX12DescriptorHeap &mDescriptorHeap;*/
        std::string mName;
    };

    struct VulkanMappedHeapAllocator {

        static constexpr size_t goodSize = 32 * 1024 * 1024; //128MB

        VulkanMappedHeapAllocator(std::string_view name);

        Block allocate(size_t size, size_t alignment = 1);
        void deallocate(Block block);

        std::pair<VkBuffer, size_t> resolve(void *ptr);

    private:
        /* D3D12_HEAP_TYPE mType;*/
        struct Page {
            VulkanPtr<VkBuffer, &vkDestroyBuffer> mBuffer;
            VulkanPtr<VkDeviceMemory, &vkFreeMemory> mMemory;
            void *mMappedAddress;
            size_t mSize;
        };
        std::vector<Page> mPages;
        std::string mName;
    };

}
}