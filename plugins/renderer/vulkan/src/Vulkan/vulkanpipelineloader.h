#pragma once

#include "Madgine/resources/resourceloader.h"

#include "Madgine/render/pipelineloader.h"

#include "util/vulkanpipeline.h"

namespace Engine {
namespace Render {

    struct VulkanPipelineInstance;

    struct MADGINE_VULKAN_EXPORT VulkanPipelineLoader : Resources::VirtualResourceLoaderImpl<VulkanPipelineLoader, VulkanPipeline, PipelineLoader> {
        VulkanPipelineLoader();

        bool loadImpl(VulkanPipeline &program, ResourceDataInfo &info);
        void unloadImpl(VulkanPipeline &program);
        Threading::Task<bool> create(Instance &instance, PipelineConfiguration config) override;

        virtual Threading::TaskQueue *loadingTaskQueue() const override;
    };
}
}

REGISTER_TYPE(Engine::Render::VulkanPipelineLoader)