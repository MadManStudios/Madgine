#pragma once

#include "Meta/math/vector2i.h"

#include "Madgine/render/texture.h"

#include "Generic/bytebuffer.h"

#include "Madgine/render/texturedescriptor.h"

namespace Engine {
namespace Render {

    struct MADGINE_VULKAN_EXPORT VulkanTexture : Texture {

        VulkanTexture(TextureType type, bool isRenderTarget, TextureFormat format, Vector2i size, size_t samples = 1, const ByteBuffer &data = {});
        VulkanTexture(TextureType type = TextureType_2D, bool isRenderTarget = false, TextureFormat format = FORMAT_RGBA8, size_t samples = 1);
        VulkanTexture(const VulkanTexture &) = delete;
        VulkanTexture(VulkanTexture &&);
        ~VulkanTexture();

        VulkanTexture &operator=(VulkanTexture &&);

        void reset();

        void setData(Vector2i size, const ByteBuffer &data);
        void setSubData(Vector2i offset, Vector2i size, const ByteBuffer &data);

        VkImageView view() const;
        VkImage image() const;

        VkFormat format() const;

        void setName(std::string_view name);

        void transition(VkCommandBuffer commandList, VkImageLayout oldLayout, VkImageLayout newLayout);

    private:
        VulkanPtr<VkDeviceMemory, &vkFreeMemory> mDeviceMemory;
        VulkanPtr<VkImageView, &vkDestroyImageView> mImageView;
        bool mIsRenderTarget;
        size_t mSamples;
    };

}
}