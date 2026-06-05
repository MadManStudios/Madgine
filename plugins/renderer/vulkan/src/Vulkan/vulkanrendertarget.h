#pragma once

#include "Madgine/render/rendertarget.h"

#include "util/vulkancommandlist.h"
#include "util/vulkantexture.h"

namespace Engine {
namespace Render {

    struct MADGINE_VULKAN_EXPORT VulkanRenderTarget : RenderTarget {

        VulkanRenderTarget(VulkanRenderContext *context, bool global, std::string name, TextureType type, size_t samples = 1, RenderTarget *blitSource = nullptr);
        ~VulkanRenderTarget();

        void createRenderPass(size_t colorAttachmentCount, VkFormat format, VkImageLayout layout, bool createDepthBufferView, std::span<VkSubpassDependency> dependencies);
        void setup(const Math::Vector2i &framebufferSize, const Math::Vector2i &size, bool createDepthBufferView = false);

        void beginFrame() override;
        RenderFuture endFrame() override;

        virtual void beginIteration(size_t targetIndex, size_t targetCount, size_t targetSubresourceIndex) const override;
        virtual void endIteration(size_t targetIndex, size_t targetCount, size_t targetSubresourceIndex) const override;

        virtual void pushAnnotation(const char *tag) override;
        virtual void popAnnotation() override;

        virtual Math::Matrix4 getClipSpaceMatrix() const override;

        size_t samples() const;

        virtual void setRenderSpace(const Math::Rect2i &space) override;
        virtual void setScissorsRect(const Math::Rect2i &space) override;

        virtual ConstTexturePtr depthTexture() const override;

        virtual void clearDepthBuffer() override;

        VulkanRenderContext *context() const;

        VulkanPtr<VkRenderPass, &vkDestroyRenderPass> mRenderPass;

        VulkanCommandList mCommandList;

        Math::Vector2i mSize;
        Math::Vector2i mBufferSize;

        VkFramebuffer mFramebuffer;

        VulkanPtr<VkSemaphore, &vkDestroySemaphore> mRenderSemaphore;

        std::shared_ptr<VulkanTexture> mDepthTexture;

        size_t mSamples;
    };

}
}