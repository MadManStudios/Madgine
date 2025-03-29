#pragma once

#include "Madgine/render/pipelineinstance.h"

#include "../vulkanpipelineloader.h"

#include "vulkanbuffer.h"

namespace Engine {
namespace Render {

    struct MADGINE_VULKAN_EXPORT VulkanPipelineInstance : PipelineInstance {

        VulkanPipelineInstance(const PipelineConfiguration &config, const VulkanPipeline *pipeline);
        ~VulkanPipelineInstance();

        bool bind(VkCommandBuffer commandList, VertexFormat format, size_t groupSize, size_t samples, VkRenderPass renderpass) const;

        virtual WritableByteBuffer mapParameters(size_t index) override;
        virtual WritableByteBuffer mapTempBuffer(size_t space, size_t size) const override;

        virtual void bindMesh(RenderTarget *target, const GPUMeshData *mesh) const override;
        virtual WritableByteBuffer mapVertices(RenderTarget *target, VertexFormat format, size_t count) const override;
        virtual ByteBufferImpl<uint32_t> mapIndices(RenderTarget *target, size_t count) const override;
        virtual void setGroupSize(size_t groupSize) const override;

        virtual void render(RenderTarget *target) const override;
        virtual void renderRange(RenderTarget *target, size_t elementCount, size_t vertexOffset, IndexType<size_t> indexOffset = {}) const override;
        virtual void renderInstanced(RenderTarget *target, size_t count) const override;

        virtual void bindResources(RenderTarget *target, size_t space, ResourceBlock block) const override;

    private:
        const VulkanPipeline *mPipeline;

        std::vector<size_t> mConstantBufferSizes;
        std::vector<uint32_t> mConstantGPUBufferOffsets;
        mutable std::vector<VkDescriptorSet> mTempDescriptors;

        bool mDepthChecking;

        VkDescriptorSet mUboDescriptorSet;        

        mutable bool mHasIndices = false;
        mutable uint32_t mElementCount;
        mutable VertexFormat mFormat;
        mutable size_t mGroupSize;
    };

    struct MADGINE_VULKAN_EXPORT VulkanPipelineInstanceHandle : VulkanPipelineInstance {

        VulkanPipelineInstanceHandle(const PipelineConfiguration &config, VulkanPipelineLoader::Handle pipeline);

    private:
        VulkanPipelineLoader::Handle mPipelineHandle;
    };

    struct MADGINE_VULKAN_EXPORT VulkanPipelineInstancePtr : VulkanPipelineInstance {

        VulkanPipelineInstancePtr(const PipelineConfiguration &config, VulkanPipelineLoader::Ptr pipeline);

    private:
        VulkanPipelineLoader::Ptr mPipelinePtr;
    };

}
}