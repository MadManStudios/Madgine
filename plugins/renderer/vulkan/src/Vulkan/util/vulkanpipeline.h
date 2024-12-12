#pragma once

#include "Madgine/pipelineloader/pipeline.h"

#include "../vulkanshaderloader.h"

#include "Madgine/render/vertexformat.h"

namespace Engine {
namespace Render {

    struct MADGINE_VULKAN_EXPORT VulkanPipeline : Pipeline {

        bool link(typename VulkanShaderLoader::Handle vertexShader, typename VulkanShaderLoader::Handle pixelShader);
        bool link(typename VulkanShaderLoader::Ptr vertexShader, typename VulkanShaderLoader::Ptr pixelShader);

        VkPipeline get(VertexFormat format, size_t groupSize, size_t samples, VkRenderPass renderpass, bool depthChecking = true) const;

        const std::array<std::array<VulkanPtr<VkPipeline, &vkDestroyPipeline>, 3>, 3> *ptr() const;

        void reset();

    private:
        mutable std::array<std::array<std::array<VulkanPtr<VkPipeline, &vkDestroyPipeline>, 3>, 3>, 256> mPipelines;

        std::variant<typename VulkanShaderLoader::Handle, typename VulkanShaderLoader::Ptr> mVertexShader;
        std::variant<typename VulkanShaderLoader::Handle, typename VulkanShaderLoader::Ptr> mPixelShader;
    };

}
}