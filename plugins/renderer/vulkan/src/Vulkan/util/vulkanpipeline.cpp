#include "../vulkanlib.h"

#include "vulkanpipeline.h"

#include "../vulkanrendercontext.h"

namespace Engine {
namespace Render {

    bool VulkanPipeline::link(typename VulkanShaderLoader::Handle vertexShader, typename VulkanShaderLoader::Handle pixelShader)
    {
        reset();

        mVertexShader = std::move(vertexShader);
        mPixelShader = std::move(pixelShader);

        return true;
    }

    bool VulkanPipeline::link(typename VulkanShaderLoader::Ptr vertexShader, typename VulkanShaderLoader::Ptr pixelShader)
    {
        reset();

        mVertexShader = std::move(vertexShader);
        mPixelShader = std::move(pixelShader);

        return true;
    }

    VkPipeline VulkanPipeline::get(VertexFormat format, size_t groupSize, size_t samples, VkRenderPass renderpass, bool depthChecking) const
    {
        size_t samplesBits = sqrt(samples);
        assert(samplesBits * samplesBits == samples);

        VulkanPtr<VkPipeline, &vkDestroyPipeline> &pipeline = mPipelines[format][groupSize - 1][samplesBits - 1];

        if (!pipeline) {

            auto check = [](auto &variant) {
                return std::visit(overloaded {
                                      [](const typename VulkanShaderLoader::Handle &handle) {
                                          return !handle || handle.available();
                                      },
                                      [](const typename VulkanShaderLoader::Ptr &ptr) {
                                          return true;
                                      } },
                    variant);
            };

            auto resolve = [](auto &variant) -> VkShaderModule {
                return std::visit(
                    [](const auto &handle) -> VkShaderModule {
                        return handle ? *handle : nullptr;
                    },
                    variant);
            };

            if (!check(mVertexShader))
                return nullptr;
            if (!check(mPixelShader))
                return nullptr;

            VkPipelineShaderStageCreateInfo shaderStages[3] = {};
            size_t count = 0;

            if (resolve(mVertexShader)) {
                VkPipelineShaderStageCreateInfo &vertShaderStageInfo = shaderStages[count++];
                vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
                vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
                vertShaderStageInfo.module = resolve(mVertexShader);
                vertShaderStageInfo.pName = "main";
            }

            if (resolve(mPixelShader)) {
                VkPipelineShaderStageCreateInfo &fragShaderStageInfo = shaderStages[count++];
                fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
                fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
                fragShaderStageInfo.module = resolve(mPixelShader);
                fragShaderStageInfo.pName = "main";
            }

            std::vector<VkDynamicState> dynamicStates = {
                VK_DYNAMIC_STATE_VIEWPORT,
                VK_DYNAMIC_STATE_SCISSOR
            };

            VkPipelineDynamicStateCreateInfo dynamicState {};
            dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
            dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
            dynamicState.pDynamicStates = dynamicStates.data();

            auto vertexLayoutDesc = VulkanRenderContext::createVertexLayout(format);

            VkPipelineVertexInputStateCreateInfo vertexInputInfo {};
            vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
            vertexInputInfo.vertexBindingDescriptionCount = vertexLayoutDesc.first.size();
            vertexInputInfo.pVertexBindingDescriptions = vertexLayoutDesc.first.data();
            vertexInputInfo.vertexAttributeDescriptionCount = vertexLayoutDesc.second.size();
            vertexInputInfo.pVertexAttributeDescriptions = vertexLayoutDesc.second.data();

            constexpr VkPrimitiveTopology types[] {
                VK_PRIMITIVE_TOPOLOGY_POINT_LIST,
                VK_PRIMITIVE_TOPOLOGY_LINE_LIST,
                VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST
            };

            VkPipelineInputAssemblyStateCreateInfo inputAssembly {};
            inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
            inputAssembly.topology = types[groupSize - 1];
            inputAssembly.primitiveRestartEnable = VK_FALSE;

            VkPipelineRasterizationStateCreateInfo rasterizer {};
            rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
            rasterizer.depthClampEnable = VK_FALSE;
            rasterizer.rasterizerDiscardEnable = VK_FALSE;
            rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
            rasterizer.lineWidth = 1.0f;
            rasterizer.cullMode = /* VK_CULL_MODE_BACK_BIT */ VK_CULL_MODE_NONE;
            rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
            rasterizer.depthBiasEnable = VK_FALSE;
            rasterizer.depthBiasConstantFactor = 0.0f; // Optional
            rasterizer.depthBiasClamp = 0.0f; // Optional
            rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

            VkPipelineMultisampleStateCreateInfo multisampling {};
            multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
            multisampling.sampleShadingEnable = VK_FALSE;
            multisampling.rasterizationSamples = static_cast<VkSampleCountFlagBits>(samples);
            multisampling.minSampleShading = 1.0f; // Optional
            multisampling.pSampleMask = nullptr; // Optional
            multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
            multisampling.alphaToOneEnable = VK_FALSE; // Optional

            VkPipelineViewportStateCreateInfo viewportState {};
            viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
            viewportState.viewportCount = 1;
            viewportState.scissorCount = 1;

            VkPipelineDepthStencilStateCreateInfo depthStencilState {};
            depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
            depthStencilState.depthTestEnable = depthChecking;
            depthStencilState.depthWriteEnable = VK_TRUE;
            depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS;
            depthStencilState.depthBoundsTestEnable = VK_FALSE;
            depthStencilState.minDepthBounds = 0.0f; // Optional
            depthStencilState.maxDepthBounds = 1.0f; // Optional
            depthStencilState.stencilTestEnable = VK_FALSE;
            depthStencilState.front = {}; // Optional
            depthStencilState.back = {}; // Optional

            VkPipelineColorBlendAttachmentState colorBlendAttachment {};
            colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
            colorBlendAttachment.blendEnable = VK_TRUE;
            colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA; // Optional
            colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA; // Optional
            colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
            colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
            colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA; // Optional
            colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

            VkPipelineColorBlendStateCreateInfo colorBlending {};
            colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
            colorBlending.logicOpEnable = VK_FALSE;
            colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
            colorBlending.attachmentCount = 1;
            colorBlending.pAttachments = &colorBlendAttachment;
            colorBlending.blendConstants[0] = 0.0f; // Optional
            colorBlending.blendConstants[1] = 0.0f; // Optional
            colorBlending.blendConstants[2] = 0.0f; // Optional
            colorBlending.blendConstants[3] = 0.0f; // Optional

            VkGraphicsPipelineCreateInfo pipelineInfo {};
            pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
            pipelineInfo.stageCount = count;
            pipelineInfo.pStages = shaderStages;
            pipelineInfo.pVertexInputState = &vertexInputInfo;
            pipelineInfo.pInputAssemblyState = &inputAssembly;
            pipelineInfo.pViewportState = &viewportState;
            pipelineInfo.pRasterizationState = &rasterizer;
            pipelineInfo.pMultisampleState = &multisampling;
            pipelineInfo.pDepthStencilState = &depthStencilState;
            pipelineInfo.pColorBlendState = &colorBlending;
            pipelineInfo.pDynamicState = &dynamicState;
            pipelineInfo.renderPass = renderpass;
            pipelineInfo.subpass = 0;
            pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
            pipelineInfo.basePipelineIndex = -1;
            pipelineInfo.layout = VulkanRenderContext::getSingleton().mPipelineLayout;
            VkResult result = vkCreateGraphicsPipelines(GetDevice(), nullptr, 1, &pipelineInfo, nullptr, &pipeline);
            VK_CHECK(result);
        }

        return pipeline;
    }

    const std::array<std::array<VulkanPtr<VkPipeline, &vkDestroyPipeline>, 3>, 3> *VulkanPipeline::ptr() const
    {
        return mPipelines.data();
    }

    void VulkanPipeline::reset()
    {
        for (std::array<std::array<VulkanPtr<VkPipeline, &vkDestroyPipeline>, 3>, 3> &groupPipelines : mPipelines)
            for (std::array<VulkanPtr<VkPipeline, &vkDestroyPipeline>, 3> &samplePipelines : groupPipelines)
                for (VulkanPtr<VkPipeline, &vkDestroyPipeline> &pipeline : samplePipelines)
                    pipeline.reset();
        mVertexShader = {};
        mPixelShader = {};
    }

}
}