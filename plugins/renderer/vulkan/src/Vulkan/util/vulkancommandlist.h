#pragma once

#include "Generic/nulledptr.h"
#include "Generic/any.h"

namespace Engine {
namespace Render {

    struct MADGINE_VULKAN_EXPORT VulkanCommandList {
        VulkanCommandList() = default;
        VulkanCommandList(NulledPtr<std::remove_pointer_t<VkCommandBuffer>> buffer, std::vector<VkSemaphore> waitSemaphores, std::vector<VkSemaphore> signalSemaphores);
        ~VulkanCommandList();

        VulkanCommandList &operator=(VulkanCommandList &&);

        RenderFuture reset();

        operator VkCommandBuffer() const;

        template <typename T>
        void attachResource(T resource)
        {
            attachResource(Any { std::move(resource) });
        }
        void attachResource(Any resource);

    private:
        NulledPtr<std::remove_pointer_t<VkCommandBuffer>> mBuffer;
        std::vector<VkSemaphore> mWaitSemaphores;
        std::vector<VkSemaphore> mSignalSemaphores;
        std::vector<Any> mAttachedResources;
    };

}
}