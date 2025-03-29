#pragma once

#include "Generic/offsetptr.h"

namespace Engine {
namespace Render {

    struct VulkanConstantBufferHeap {
        VulkanConstantBufferHeap() = default;
        VulkanConstantBufferHeap(size_t size);

        VulkanConstantBufferHeap &operator=(VulkanConstantBufferHeap &&other);

        void reset();

        OffsetPtr allocateTemp(size_t size);
        void deallocateTemp(OffsetPtr ptr, size_t size);
        OffsetPtr allocatePersistent(size_t size);
        void deallocatePersistent(OffsetPtr ptr, size_t size);

        WritableByteBuffer map(OffsetPtr ptr, size_t size);
        void setData(OffsetPtr ptr, const ByteBuffer &data);

        VkBuffer resourceTemp() const;
        VkBuffer resourcePersistent() const;

    private:
        VulkanPtr<VkDeviceMemory, &vkFreeMemory> mTempMemory;
        VulkanPtr<VkBuffer, &vkDestroyBuffer> mTempHeap;
        VulkanPtr<VkDeviceMemory, &vkFreeMemory> mPersistentMemory;
        VulkanPtr<VkBuffer, &vkDestroyBuffer> mPersistentHeap;
        size_t mSize = 0;
        OffsetPtr mOffsetTemp { 0 };
        OffsetPtr mOffsetPersistent { 0 };

        //std::vector<std::pair<OffsetPtr, size_t>> mFreeListTemp;
        std::vector<std::pair<OffsetPtr, size_t>> mFreeListPersistent;
    };

}
}