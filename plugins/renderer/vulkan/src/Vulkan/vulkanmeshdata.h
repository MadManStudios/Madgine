#pragma once

#include "Madgine/meshloader/gpumeshdata.h"

#include "util/vulkanbuffer.h"
#include "util/vulkantexture.h"

namespace Engine {
namespace Render {

    struct MADGINE_VULKAN_EXPORT VulkanMeshData : GPUMeshData {
        VulkanBuffer mVertices;
        VulkanBuffer mIndices;

        std::vector<TexturePtr> mTextureCache;
    };

}
}