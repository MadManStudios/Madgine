#pragma once

#include "util/vulkantexture.h"
#include "vulkanrendertarget.h"

namespace Engine {
namespace Render {

    struct MADGINE_VULKAN_EXPORT VulkanRenderTexture : VulkanRenderTarget {

        VulkanRenderTexture(VulkanRenderContext *context, const Vector2i &size, const RenderTextureConfig &config);
        ~VulkanRenderTexture();

        bool resizeImpl(const Vector2i &size) override;
        Vector2i size() const override;

        virtual void beginFrame() override;
        virtual RenderFuture endFrame() override;

        virtual void beginIteration(size_t targetIndex, size_t targetCount, size_t targetSubresourceIndex) const override;
        virtual void endIteration(size_t targetIndex, size_t targetCount, size_t targetSubresourceIndex) const override;

        virtual const Texture*texture(size_t index) const override;
        virtual size_t textureCount() const override;

        void blit(RenderTarget *input);

        const std::vector<VulkanTexture> &textures() const;

    protected:
        void createRenderPass();

    private:
        std::vector<VulkanTexture> mTextures;

        Vector2i mSize;

        VulkanPtr<VkFramebuffer, &vkDestroyFramebuffer> mTextureFramebuffer;

        bool mCreateDepthBufferView;
    };

}
}