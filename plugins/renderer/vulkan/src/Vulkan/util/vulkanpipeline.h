#pragma once

#include "Madgine/render/pipeline.h"
#include "Madgine/render/vertexformat.h"

#include "../vulkanshaderloader.h"

namespace Engine {
namespace Render {

    struct MADGINE_VULKAN_EXPORT VulkanPipeline : Pipeline {

        bool link(PipelineSignature signature, typename VulkanShaderLoader::Handle vertexShader, std::string vs_entrypoint, typename VulkanShaderLoader::Handle pixelShader, std::string ps_entrypoint);

        VkPipeline get(VertexFormat format, size_t groupSize, size_t samples, VkRenderPass renderpass, bool depthChecking = true, bool depthWriting = true) const;

        const std::array<std::array<VulkanPtr<VkPipeline, &vkDestroyPipeline>, 3>, 3> *ptr() const;

        void reset();

        VkPipelineLayout layout() const;

    private:
        mutable std::array<std::array<std::array<VulkanPtr<VkPipeline, &vkDestroyPipeline>, 3>, 3>, 256> mPipelines;
        
        VkPipelineLayout mLayout;

        typename VulkanShaderLoader::Handle mVertexShader;
        std::string mVsEntrypoint;
        typename VulkanShaderLoader::Handle mPixelShader;
        std::string mPsEntrypoint;
    };

}
}