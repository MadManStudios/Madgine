#include "../vulkanlib.h"

#include "vulkanbuffer.h"

#include "../vulkanrendercontext.h"

#include "Generic/bytebuffer.h"

#include "Generic/align.h"

#include "vulkanconstantbufferheap.h"

namespace Engine {
namespace Render {

    VulkanBuffer::VulkanBuffer(VulkanBuffer &&other)
        : mBlock(std::exchange(other.mBlock, {}))
    {
    }

    VulkanBuffer::VulkanBuffer(const ByteBuffer &data)
    {
        setData(data);
    }

    VulkanBuffer::~VulkanBuffer()
    {
        reset();
    }

    VulkanBuffer &VulkanBuffer::operator=(VulkanBuffer &&other)
    {
        std::swap(mBlock, other.mBlock);
        return *this;
    }

    VulkanBuffer::operator bool() const
    {
        return mBlock.mAddress;
    }

    void VulkanBuffer::bindVertex(VkCommandBuffer commandList, size_t index) const
    {
        auto [buffer, offset] = VulkanRenderContext::getSingleton().mConstantMemoryHeap.resolve(mBlock.mAddress);
        VkDeviceSize vOffset = offset;
        vkCmdBindVertexBuffers(commandList, index, 1, &buffer, &vOffset);
    }

    void VulkanBuffer::bindIndex(VkCommandBuffer commandList) const
    {
        auto [buffer, offset] = VulkanRenderContext::getSingleton().mConstantMemoryHeap.resolve(mBlock.mAddress);
        vkCmdBindIndexBuffer(commandList, buffer, offset, VK_INDEX_TYPE_UINT32);
    }

    void VulkanBuffer::reset()
    {
        if (mBlock.mAddress) {
            VulkanRenderContext::getSingleton().mConstantAllocator.deallocate(mBlock);
            mBlock = {};
        }
    }

    void VulkanBuffer::setData(const ByteBuffer &data)
    {
        assert(data.mData);

        VulkanRenderContext &context = VulkanRenderContext::getSingleton();

        size_t expectedSize = alignTo(data.mSize, 256);
        if (mBlock.mSize != expectedSize) {
            reset();
            mBlock = context.mConstantAllocator.allocate(expectedSize);
        }

        Block uploadAllocation = context.mUploadAllocator.allocate(expectedSize);
        std::memcpy(uploadAllocation.mAddress, data.mData, data.mSize);

        auto [buffer, offset] = context.mConstantMemoryHeap.resolve(mBlock.mAddress);
        auto [heap, srcOffset] = context.mUploadHeap.resolve(uploadAllocation.mAddress);

        auto list = context.fetchCommandList("BufferUpload");

        VkBufferCopy copyRegion {};
        copyRegion.srcOffset = srcOffset; // Optional
        copyRegion.dstOffset = offset; // Optional
        copyRegion.size = data.mSize;
        vkCmdCopyBuffer(list, heap, buffer, 1, &copyRegion);
        
        struct Deleter {

            void operator()(void *ptr)
            {
                VulkanRenderContext::getSingleton().mUploadAllocator.deallocate({ ptr, mSize });
            }

            size_t mSize;
        };
        list.attachResource(std::unique_ptr<void, Deleter> { uploadAllocation.mAddress, { uploadAllocation.mSize } });
    }

    size_t VulkanBuffer::offset() const
    {
        return VulkanRenderContext::getSingleton().mConstantMemoryHeap.resolve(mBlock.mAddress).second;
    }

    VkBuffer VulkanBuffer::buffer() const
    {
        return VulkanRenderContext::getSingleton().mConstantMemoryHeap.resolve(mBlock.mAddress).first;
    }

    size_t VulkanBuffer::size() const
    {
        return mBlock.mSize;
    }

}
}