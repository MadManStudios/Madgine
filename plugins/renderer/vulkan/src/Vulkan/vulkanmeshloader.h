#pragma once

#include "Madgine/resources/resourceloader.h"

#include "Madgine/meshloader/gpumeshloader.h"

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

REGISTER_TYPE(Engine::Render::VulkanMeshLoader)