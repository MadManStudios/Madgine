#pragma once

#include "util/vulkanbuffer.h"
#include "util/vulkantexture.h"
#include "Madgine/render/textureloader.h"

#include "Madgine/meshloader/gpumeshdata.h"

namespace Engine {
namespace Render {

    struct MADGINE_VULKAN_EXPORT VulkanMeshData : GPUMeshData {
        VulkanBuffer mVertices;
        VulkanBuffer mIndices;

        std::vector<TextureLoader::Handle> mTextureCache;
    };

}
}