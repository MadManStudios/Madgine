#pragma once

#include "Generic/bytebuffer.h"

#include "Meta/math/vector2i.h"

#include "Madgine/render/texture.h"
#include "Madgine/render/texturedescriptor.h"

namespace Engine {
namespace Render {
        
    template <size_t I = 1>
    struct VulkanResourceBlock {
        VkDescriptorSet mHandle;
        size_t mSize = I;
        std::variant<ConstTexturePtr, GPUPtr<void>, GPUPtr<Void[]>> mResources[I];
    };


    struct MADGINE_VULKAN_EXPORT VulkanTexture : Texture {

        VulkanTexture(TextureType type, bool isRenderTarget, TextureFormat format, Vector2i size, size_t samples = 1, const ByteBuffer &data = {});
        VulkanTexture(TextureType type = TextureType_2D, bool isRenderTarget = false, TextureFormat format = FORMAT_RGBA8, size_t samples = 1);
        VulkanTexture(const VulkanTexture &) = delete;
        VulkanTexture(VulkanTexture &&);
        ~VulkanTexture();

        VulkanTexture &operator=(VulkanTexture &&);

        void reset();

        void setSubData(Vector2i offset, Vector2i size, const ByteBuffer &data);

        VkImageView view() const;
        VkImage image() const;

        VkFormat vkFormat() const;
        TextureFormat format() const;

        size_t samples() const;

        void setName(std::string_view name);

        void transition(VkCommandBuffer commandList, VkImageLayout oldLayout, VkImageLayout newLayout);

    private:
        VulkanPtr<VkDeviceMemory, &vkFreeMemory> mDeviceMemory;
        VulkanPtr<VkImageView, &vkDestroyImageView> mImageView;
        bool mIsRenderTarget;
        size_t mSamples;
        VulkanResourceBlock<1> mBlock;
    };

}
}