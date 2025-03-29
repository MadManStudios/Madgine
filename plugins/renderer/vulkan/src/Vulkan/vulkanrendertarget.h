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
        void setup(const Vector2i &size, bool createDepthBufferView = false);
        void shutdown();

        void beginFrame() override;
        RenderFuture endFrame() override;

        virtual void beginIteration(size_t targetIndex, size_t targetCount, size_t targetSubresourceIndex) const override;
        virtual void endIteration(size_t targetIndex, size_t targetCount, size_t targetSubresourceIndex) const override;

        virtual void pushAnnotation(const char *tag) override;
        virtual void popAnnotation() override;

        virtual Matrix4 getClipSpaceMatrix() const override;

        size_t samples() const;

        virtual void setRenderSpace(const Rect2i &space) override;
        virtual void setScissorsRect(const Rect2i &space) override;

        virtual const Texture *depthTexture() const override;

        virtual void clearDepthBuffer() override;

        VulkanRenderContext *context() const;

        VulkanPtr<VkRenderPass, &vkDestroyRenderPass> mRenderPass;

        VulkanCommandList mCommandList;

        Vector2i mSize;

        VkFramebuffer mFramebuffer;

        VulkanPtr<VkSemaphore, &vkDestroySemaphore> mRenderSemaphore;

        VulkanTexture mDepthTexture;

        size_t mSamples;
    };

}
}