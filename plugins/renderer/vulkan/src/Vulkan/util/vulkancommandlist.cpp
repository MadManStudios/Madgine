#include "../vulkanlib.h"

#include "../vulkanrendercontext.h"
#include "../vulkanrendertarget.h"
#include "vulkancommandlist.h"

namespace Engine {
namespace Render {

    VulkanCommandList::VulkanCommandList(NulledPtr<std::remove_pointer_t<VkCommandBuffer>> buffer, std::vector<VkSemaphore> waitSemaphores, std::vector<VkSemaphore> signalSemaphores)
        : mBuffer(std::move(buffer))
        , mWaitSemaphores(std::move(waitSemaphores))
        , mSignalSemaphores(std::move(signalSemaphores))        
    {
    }

    VulkanCommandList::~VulkanCommandList()
    {
        reset();
    }

    VulkanCommandList &VulkanCommandList::operator=(VulkanCommandList &&other)
    {
        reset();
        mBuffer = std::move(other.mBuffer);
        std::swap(mWaitSemaphores, other.mWaitSemaphores);
        std::swap(mSignalSemaphores, other.mSignalSemaphores);
        std::swap(mAttachedResources, other.mAttachedResources);
        return *this;
    }

    RenderFuture VulkanCommandList::reset()
    {
        if (mBuffer) {
            return VulkanRenderContext::getSingleton().ExecuteCommandList(std::move(mBuffer), std::move(mWaitSemaphores), std::move(mSignalSemaphores), std::move(mAttachedResources));
        }
        return {};
    }

    VulkanCommandList::operator VkCommandBuffer() const
    {
        return mBuffer;
    }

    void VulkanCommandList::attachResource(Any resource)
    {
        mAttachedResources.push_back(std::move(resource));
    }

}
}