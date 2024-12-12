#include "../vulkanlib.h"

#include "vulkanpipelineinstance.h"

#include "../vulkanrendercontext.h"

#include "../vulkanmeshdata.h"
#include "../vulkanrendertarget.h"

namespace Engine {
namespace Render {

    VulkanPipelineInstance::VulkanPipelineInstance(const PipelineConfiguration &config, const VulkanPipeline *pipeline)
        : PipelineInstance(config)
        , mPipeline(pipeline)
        , mConstantBufferSizes(config.bufferSizes)
        , mDepthChecking(config.depthChecking)
    {
        mConstantGPUBufferOffsets.resize(3);

        VulkanRenderContext &context = VulkanRenderContext::getSingleton();

        VkDescriptorSetAllocateInfo allocInfo {};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = context.mDescriptorPool;
        allocInfo.descriptorSetCount = 1;
        allocInfo.pSetLayouts = &std::as_const(context.mUBODescriptorSetLayout);
        VkResult result = vkAllocateDescriptorSets(GetDevice(), &allocInfo, &mUboDescriptorSet);
        VK_CHECK(result);

        VkDebugUtilsObjectNameInfoEXT nameInfo {};
        nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
        nameInfo.objectType = VK_OBJECT_TYPE_DESCRIPTOR_SET;
        nameInfo.objectHandle = reinterpret_cast<uintptr_t>(mUboDescriptorSet);
        nameInfo.pObjectName = "Pipeline UBO Set";
        result = vkSetDebugUtilsObjectNameEXT(GetDevice(), &nameInfo);
        VK_CHECK(result);

        context.mTempAllocator.allocate(1);
        Block block = context.mTempAllocator.parent().parent().getBlock();
        VkBuffer buffer = context.mTempMemoryHeap.resolve(block.mAddress).first;

        for (size_t i = 0; i < std::min(config.bufferSizes.size(), size_t { 3 }); ++i) {
            if (config.bufferSizes[i] == 0)
                continue;
            VkDescriptorBufferInfo bufferInfo {};
            bufferInfo.buffer = buffer;
            bufferInfo.offset = 0;
            bufferInfo.range = config.bufferSizes[i];

            VkWriteDescriptorSet descriptorWrite {};
            descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrite.dstSet = mUboDescriptorSet;
            descriptorWrite.dstBinding = i;
            descriptorWrite.dstArrayElement = 0;

            descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
            descriptorWrite.descriptorCount = 1;

            descriptorWrite.pBufferInfo = &bufferInfo;
            descriptorWrite.pImageInfo = nullptr; // Optional
            descriptorWrite.pTexelBufferView = nullptr; // Optional

            vkUpdateDescriptorSets(GetDevice(), 1, &descriptorWrite, 0, nullptr);
        }
    }

    VulkanPipelineInstance::~VulkanPipelineInstance()
    {
        VkResult result = vkFreeDescriptorSets(GetDevice(), VulkanRenderContext::getSingleton().mDescriptorPool, 1, &mUboDescriptorSet);
        VK_CHECK(result);
    }

    bool VulkanPipelineInstance::bind(VkCommandBuffer commandList, VertexFormat format, size_t groupSize, size_t samples, VkRenderPass renderpass) const
    {
        size_t samplesBits = sqrt(samples);
        assert(samplesBits * samplesBits == samples);

        VkPipeline pipeline = mPipeline->ptr()[format][groupSize - 1][samplesBits - 1];
        if (!pipeline) {
            pipeline = mPipeline->get(format, groupSize, samples, renderpass, mDepthChecking);
            if (!pipeline)
                return false;
        }
        vkCmdBindPipeline(commandList, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

        vkCmdBindDescriptorSets(commandList, VK_PIPELINE_BIND_POINT_GRAPHICS, VulkanRenderContext::getSingleton().mPipelineLayout, 0, 1, &mUboDescriptorSet, mConstantGPUBufferOffsets.size(), mConstantGPUBufferOffsets.data());

        if (!mTempDescriptors.empty())
            vkCmdBindDescriptorSets(commandList, VK_PIPELINE_BIND_POINT_GRAPHICS, VulkanRenderContext::getSingleton().mPipelineLayout, 2, mTempDescriptors.size(), mTempDescriptors.data(), 0, nullptr);

        return true;
    }

    WritableByteBuffer VulkanPipelineInstance::mapParameters(size_t index)
    {
        Block block = VulkanRenderContext::getSingleton().mTempAllocator.allocate(mConstantBufferSizes[index], 256);
        mConstantGPUBufferOffsets[index] = VulkanRenderContext::getSingleton().mTempMemoryHeap.resolve(block.mAddress).second;

        return { block.mAddress, block.mSize };
    }

    void VulkanPipelineInstance::render(RenderTarget *_target) const
    {
        VulkanRenderTarget *target = static_cast<VulkanRenderTarget *>(_target);
        VkCommandBuffer commandList = target->mCommandList;

        if (!bind(commandList, mFormat, mGroupSize, target->samples(), target->mRenderPass))
            return;

        if (mHasIndices) {
            vkCmdDrawIndexed(commandList, mElementCount, 1, 0, 0, 0);
        } else {
            vkCmdDraw(commandList, mElementCount, 1, 0, 0);
        }

        mHasIndices = false;
    }

    void VulkanPipelineInstance::renderRange(RenderTarget *_target, size_t elementCount, size_t vertexOffset, IndexType<size_t> indexOffset) const
    {
        VulkanRenderTarget *target = static_cast<VulkanRenderTarget *>(_target);
        VkCommandBuffer commandList = target->mCommandList;

        if (!bind(commandList, mFormat, mGroupSize, target->samples(), target->mRenderPass))
            return;

        if (mHasIndices) {
            assert(indexOffset);
            vkCmdDrawIndexed(commandList, elementCount, 1, indexOffset, vertexOffset, 0);
        } else {
            vkCmdDraw(commandList, elementCount, 1, vertexOffset, 0);
        }
    }

    void VulkanPipelineInstance::renderInstanced(RenderTarget *_target, size_t count) const
    {
        VulkanRenderTarget *target = static_cast<VulkanRenderTarget *>(_target);
        VkCommandBuffer commandList = target->mCommandList;

        if (!bind(commandList, mFormat, mGroupSize, target->samples(), target->mRenderPass))
            return;

        if (mHasIndices) {
            vkCmdDrawIndexed(commandList, mElementCount, count, 0, 0, 0);
        } else {
            vkCmdDraw(commandList, mElementCount, count, 0, 0);
        }

        mHasIndices = false;
    }

    WritableByteBuffer VulkanPipelineInstance::mapTempBuffer(size_t space, size_t size) const
    {
        assert(space >= 1);
        if (mTempDescriptors.size() <= space - 1)
            mTempDescriptors.resize(space);

        VkDescriptorSet set = VulkanRenderContext::getSingleton().fetchTempDescriptorSet();

        Block block = VulkanRenderContext::getSingleton().mTempAllocator.allocate(size, 256);
        auto [buffer, offset] = VulkanRenderContext::getSingleton().mTempMemoryHeap.resolve(block.mAddress);

        VkDescriptorBufferInfo bufferInfo {};
        bufferInfo.buffer = buffer;
        bufferInfo.offset = offset;
        bufferInfo.range = size;        

        VkWriteDescriptorSet descriptorWrite {};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = set;
        descriptorWrite.dstBinding = space - 1;
        descriptorWrite.dstArrayElement = 0;

        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        descriptorWrite.descriptorCount = 1;

        descriptorWrite.pBufferInfo = &bufferInfo;
        descriptorWrite.pImageInfo = nullptr; // Optional
        descriptorWrite.pTexelBufferView = nullptr; // Optional

        vkUpdateDescriptorSets(GetDevice(), 1, &descriptorWrite, 0, nullptr);

        mTempDescriptors[space - 1] = set;

        return { block.mAddress, block.mSize };
    }

    void VulkanPipelineInstance::bindMesh(RenderTarget *target, const GPUMeshData *m) const
    {
        VkCommandBuffer commandList = static_cast<VulkanRenderTarget *>(target)->mCommandList;
        VkRenderPass renderpass = static_cast<VulkanRenderTarget *>(target)->mRenderPass;

        const VulkanMeshData *mesh = static_cast<const VulkanMeshData *>(m);

        mFormat = mesh->mFormat;
        mGroupSize = mesh->mGroupSize;

        mesh->mVertices.bindVertex(commandList);

        VulkanRenderContext::getSingleton().mConstantBuffer.bindVertex(commandList, 2);

        if (mesh->mIndices) {
            mesh->mIndices.bindIndex(commandList);
            mHasIndices = true;
        } else {
            mHasIndices = false;
        }

        mElementCount = mesh->mElementCount;
    }

    WritableByteBuffer VulkanPipelineInstance::mapVertices(RenderTarget *_target, VertexFormat format, size_t count) const
    {
        VulkanRenderTarget *target = static_cast<VulkanRenderTarget *>(_target);
        VkCommandBuffer commandList = target->mCommandList;

        Block block = VulkanRenderContext::getSingleton().mTempAllocator.allocate(format.stride() * count);
        auto [buffer, offset] = VulkanRenderContext::getSingleton().mTempMemoryHeap.resolve(block.mAddress);
        VkDeviceSize vOffset = offset;

        vkCmdBindVertexBuffers(commandList, 0, 1, &buffer, &vOffset);

        VulkanRenderContext::getSingleton().mConstantBuffer.bindVertex(commandList, 2);

        mFormat = format;
        mElementCount = count;

        return { block.mAddress, block.mSize };
    }

    ByteBufferImpl<uint32_t> VulkanPipelineInstance::mapIndices(RenderTarget *_target, size_t count) const
    {
        VulkanRenderTarget *target = static_cast<VulkanRenderTarget *>(_target);

        Block block = VulkanRenderContext::getSingleton().mTempAllocator.allocate(sizeof(uint32_t) * count);
        auto [buffer, offset] = VulkanRenderContext::getSingleton().mTempMemoryHeap.resolve(block.mAddress);

        vkCmdBindIndexBuffer(target->mCommandList, buffer, offset, VK_INDEX_TYPE_UINT32);

        mHasIndices = true;
        mElementCount = count;

        return { static_cast<uint32_t *>(block.mAddress), block.mSize };
    }

    void VulkanPipelineInstance::setGroupSize(size_t groupSize) const
    {
        mGroupSize = groupSize;
    }

    void VulkanPipelineInstance::bindResources(RenderTarget *_target, size_t space, ResourceBlock block) const
    {
        assert(space > 1);
        VulkanRenderTarget *target = static_cast<VulkanRenderTarget *>(_target);
        VkCommandBuffer commandList = target->mCommandList;
        VkDescriptorSet set;
        if (block) {
            set = block;
        } else {
            set = target->context()->mDefaultResourceBlockDescriptorSet;
        }
        vkCmdBindDescriptorSets(commandList, VK_PIPELINE_BIND_POINT_GRAPHICS, target->context()->mPipelineLayout, 1 + space, 1, &set, 0, nullptr);
    }

    VulkanPipelineInstanceHandle::VulkanPipelineInstanceHandle(const PipelineConfiguration &config, VulkanPipelineLoader::Handle pipeline)
        : VulkanPipelineInstance(config, &*pipeline)
        , mPipelineHandle(std::move(pipeline))
    {
    }

    VulkanPipelineInstancePtr::VulkanPipelineInstancePtr(const PipelineConfiguration &config, VulkanPipelineLoader::Ptr pipeline)
        : VulkanPipelineInstance(config, &*pipeline)
        , mPipelinePtr(std::move(pipeline))
    {
    }

}
}