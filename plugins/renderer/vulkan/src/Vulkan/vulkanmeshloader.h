#pragma once

#include "Madgine/meshloader/gpumeshloader.h"
#include "Madgine/resources/resourceloader.h"

#include "vulkanmeshdata.h"

namespace Engine {
namespace Render {

    struct MADGINE_VULKAN_EXPORT VulkanMeshLoader : Resources::VirtualResourceLoaderImpl<VulkanMeshLoader, VulkanMeshData, GPUMeshLoader> {
        VulkanMeshLoader();

        virtual Threading::Task<bool> generate(GPUMeshData &data, const MeshData &mesh) override;

        virtual void reset(GPUMeshData &data) override;

        virtual Threading::TaskQueue *loadingTaskQueue() const override;
    };
}
}