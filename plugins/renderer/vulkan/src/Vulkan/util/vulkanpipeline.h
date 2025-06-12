#pragma once

#include "Madgine/render/pipeline.h"

#include "../vulkanshaderloader.h"

#include "Madgine/render/vertexformat.h"

namespace Engine {
namespace Render {

    struct MADGINE_VULKAN_EXPORT VulkanPipeline : Pipeline {

        bool link(typename VulkanShaderLoader::Handle vertexShader, std::string vs_entrypoint, typename VulkanShaderLoader::Handle pixelShader, std::string ps_entrypoint);
        bool link(typename VulkanShaderLoader::Ptr vertexShader, std::string vs_entrypoint, typename VulkanShaderLoader::Ptr pixelShader, std::string ps_entrypoint);

        VkPipeline get(VertexFormat format, size_t groupSize, size_t samples, VkRenderPass renderpass, bool depthChecking = true) const;

        const std::array<std::array<VulkanPtr<VkPipeline, &vkDestroyPipeline>, 3>, 3> *ptr() const;

        void reset();

    private:
        mutable std::array<std::array<std::array<VulkanPtr<VkPipeline, &vkDestroyPipeline>, 3>, 3>, 256> mPipelines;

        std::variant<typename VulkanShaderLoader::Handle, typename VulkanShaderLoader::Ptr> mVertexShader;
        std::string mVsEntrypoint;
        std::variant<typename VulkanShaderLoader::Handle, typename VulkanShaderLoader::Ptr> mPixelShader;
        std::string mPsEntrypoint;
    };

}
}