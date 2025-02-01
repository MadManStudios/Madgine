#pragma once

#include "Madgine/resources/resourceloader.h"

#include "Madgine/render/textureloader.h"

#include "util/vulkantexture.h"

namespace Engine {
namespace Render {

    struct MADGINE_VULKAN_EXPORT VulkanTextureLoader : Resources::VirtualResourceLoaderImpl<VulkanTextureLoader, VulkanTexture, TextureLoader> {
        VulkanTextureLoader();

        bool loadImpl(VulkanTexture &tex, ResourceDataInfo &info);
        void unloadImpl(VulkanTexture &tex);
        bool create(Texture &texture, TextureType type, TextureFormat format) override;

        virtual void setData(Texture &tex, Vector2i size, const ByteBuffer &data) override;
        virtual void setSubData(Texture &tex, Vector2i offset, Vector2i size, const ByteBuffer &data) override;

        virtual Threading::TaskQueue *loadingTaskQueue() const override;
    };
}
}

REGISTER_TYPE(Engine::Render::VulkanTextureLoader)
