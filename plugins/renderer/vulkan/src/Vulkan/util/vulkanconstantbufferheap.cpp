#include "../vulkanlib.h"

#include "vulkanconstantbufferheap.h"

#include "../vulkanrendercontext.h"

#include "Generic/bytebuffer.h"

namespace Engine {
namespace Render {

    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
    {
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(GetPhysicalDevice(), &memProperties);

        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
            if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
                return i;
            }
        }

        throw 0;
    }

    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer *buffer, VkDeviceMemory *bufferMemory)
    {
        VkBufferCreateInfo bufferInfo {};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;
        bufferInfo.usage = usage;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateBuffer(GetDevice(), &bufferInfo, nullptr, buffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to create buffer!");
        }

        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(GetDevice(), *buffer, &memRequirements);

        VkMemoryAllocateInfo allocInfo {};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

        if (vkAllocateMemory(GetDevice(), &allocInfo, nullptr, bufferMemory) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate buffer memory!");
        }

        vkBindBufferMemory(GetDevice(), *buffer, *bufferMemory, 0);
    }

    VulkanConstantBufferHeap::VulkanConstantBufferHeap(size_t size)
        : mSize(size)
    {
        createBuffer(size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &mTempHeap, &mTempMemory);

        createBuffer(size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &mPersistentHeap, &mPersistentMemory);
    }

    VulkanConstantBufferHeap &VulkanConstantBufferHeap::operator=(VulkanConstantBufferHeap &&other)
    {
        std::swap(mTempMemory, other.mTempMemory);
        std::swap(mTempHeap, other.mTempHeap);
        std::swap(mPersistentMemory, other.mPersistentMemory);
        std::swap(mPersistentHeap, other.mPersistentHeap);
        std::swap(mSize, other.mSize);
        std::swap(mOffsetTemp, other.mOffsetTemp);
        std::swap(mOffsetPersistent, other.mOffsetPersistent);
        return *this;
    }

    void VulkanConstantBufferHeap::reset()
    {
        mTempHeap.reset();
        mTempMemory.reset();
        mPersistentHeap.reset();
        mPersistentMemory.reset();
    }

    OffsetPtr VulkanConstantBufferHeap::allocateTemp(size_t size)
    {
        /* for (auto it = mFreeListTemp.begin(); it != mFreeListTemp.end(); ++it) {
            if (it->second == size) {
                OffsetPtr offset = it->first;
                mFreeListTemp.erase(it);
                return offset;
            } else if (it->second > size) {
                it->second -= size;
                return it->first + it->second;
            }
        }
        */
        if (mOffsetTemp + size > mSize) {
            mOffsetTemp = OffsetPtr { 0 };
        }
        OffsetPtr offset = mOffsetTemp;
        mOffsetTemp += size;
        return offset;
    }

    void VulkanConstantBufferHeap::deallocateTemp(OffsetPtr ptr, size_t size)
    {
        //mFreeListTemp.push_back({ ptr, size });
    }

    OffsetPtr VulkanConstantBufferHeap::allocatePersistent(size_t size)
    {
        for (auto it = mFreeListPersistent.begin(); it != mFreeListPersistent.end(); ++it) {
            if (it->second == size) {
                OffsetPtr ptr = it->first;
                mFreeListPersistent.erase(it);
                return ptr;
            } else if (it->second > size) {
                it->second -= size;
                return it->first + it->second;
            }
        }

        assert(mOffsetPersistent + size <= mSize);
        OffsetPtr offset = mOffsetPersistent;
        mOffsetPersistent += size;
        return offset;
    }

    void VulkanConstantBufferHeap::deallocatePersistent(OffsetPtr ptr, size_t size)
    {
        mFreeListPersistent.push_back({ ptr, size });
    }

    WritableByteBuffer VulkanConstantBufferHeap::map(OffsetPtr ptr, size_t size)
    {
        struct UnmapDeleter {
            VkDeviceMemory mMemory;

            void operator()(void *p)
            {
                vkUnmapMemory(GetDevice(), mMemory);
            }
        };

        void *data;
        VkResult result = vkMapMemory(GetDevice(), mTempMemory, ptr.offset(), size, 0, &data);
        VK_CHECK(result);

        std::unique_ptr<void, UnmapDeleter> dataBuffer { data, { mTempMemory } };

        return { std::move(dataBuffer), size };
    }

    void VulkanConstantBufferHeap::setData(OffsetPtr ptr, const ByteBuffer &data)
    {
        VulkanPtr<VkDeviceMemory, &vkFreeMemory> stagingBufferMemory;
        VulkanPtr<VkBuffer, &vkDestroyBuffer> stagingBuffer;
        createBuffer(data.mSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingBuffer, &stagingBufferMemory);

        void *dest;
        VkResult result = vkMapMemory(GetDevice(), stagingBufferMemory, 0, data.mSize, 0, &dest);
        VK_CHECK(result);

        memcpy(dest, data.mData, data.mSize);

        vkUnmapMemory(GetDevice(), stagingBufferMemory);

        VulkanRenderContext &context = VulkanRenderContext::getSingleton();

        VkCommandBufferAllocateInfo allocInfo {};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = context.mCommandPool;
        allocInfo.commandBufferCount = 1;

        VkCommandBuffer commandBuffer;
        vkAllocateCommandBuffers(GetDevice(), &allocInfo, &commandBuffer);

        VkCommandBufferBeginInfo beginInfo {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(commandBuffer, &beginInfo);

        VkBufferCopy copyRegion {};
        copyRegion.srcOffset = 0; // Optional
        copyRegion.dstOffset = ptr.offset(); // Optional
        copyRegion.size = data.mSize;
        vkCmdCopyBuffer(commandBuffer, stagingBuffer, mPersistentHeap, 1, &copyRegion);

        vkEndCommandBuffer(commandBuffer);

        VkSubmitInfo submitInfo {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        vkQueueSubmit(context.mGraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(context.mGraphicsQueue);

        vkFreeCommandBuffers(GetDevice(), context.mCommandPool, 1, &commandBuffer);
    }

    VkBuffer VulkanConstantBufferHeap::resourceTemp() const
    {
        return mTempHeap;
    }

    VkBuffer VulkanConstantBufferHeap::resourcePersistent() const
    {
        return mPersistentHeap;
    }

}
}