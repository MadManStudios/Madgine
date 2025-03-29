#pragma once

#include "Generic/offsetptr.h"

#include "Generic/allocator/concepts.h"

namespace Engine {
namespace Render {

    struct MADGINE_VULKAN_EXPORT VulkanBuffer {

        VulkanBuffer() = default;
        VulkanBuffer(const ByteBuffer &data);
        VulkanBuffer(const VulkanBuffer &) = delete;
        VulkanBuffer(VulkanBuffer &&);
        ~VulkanBuffer();

        VulkanBuffer &operator=(const VulkanBuffer &) = delete;
        VulkanBuffer &operator=(VulkanBuffer &&);

        explicit operator bool() const;

        void bindVertex(VkCommandBuffer commandList, size_t index = 0) const;
        void bindIndex(VkCommandBuffer commandList) const;

        void reset();
        void setData(const ByteBuffer &data);

        size_t offset() const;
        VkBuffer buffer() const;

        size_t size() const;

    private:
        Block mBlock;
    };

}
}