#pragma once

#include "Madgine/render/rendercontext.h"
#include "Madgine/render/rendercontextcollector.h"

#include "Modules/threading/threadlocal.h"

#include "util/vulkancommandlist.h"

#include "util/vulkanbuffer.h"
#include "util/vulkantexture.h"

#include "Madgine/render/vertexformat.h"

#include "util/vulkanheapallocator.h"

#include "Generic/allocator/bucket.h"
#include "Generic/allocator/bump.h"
#include "Generic/allocator/fixed.h"
#include "Generic/allocator/heap.h"
#include "Generic/allocator/tracked.h"

namespace Engine {
namespace Render {

    struct QueueFamilyIndices {
        std::optional<uint32_t> graphicsFamily;

        bool isComplete()
        {
            return graphicsFamily.has_value();
        }
    };

    MADGINE_VULKAN_EXPORT VkDevice GetDevice();
    MADGINE_VULKAN_EXPORT VkInstance GetInstance();
    MADGINE_VULKAN_EXPORT VkPhysicalDevice GetPhysicalDevice();

    struct MADGINE_VULKAN_EXPORT VulkanRenderContext : public RenderContextComponent<VulkanRenderContext> {
        VulkanRenderContext(Threading::TaskQueue *queue);
        VulkanRenderContext(const VulkanRenderContext &) = delete;
        ~VulkanRenderContext();

        virtual std::unique_ptr<RenderTarget> createRenderWindow(Window::OSWindow *w, size_t samples) override;
        virtual std::unique_ptr<RenderTarget> createRenderTexture(const Vector2i &size = { 1, 1 }, const RenderTextureConfig &config = {}) override;

        virtual bool beginFrame() override;
        virtual void endFrame() override;

        VkDescriptorSet allocateResourceBlock(std::vector<const Texture *> textures);
        void freeResourceBlock(VkDescriptorSet descriptorSet);
        virtual UniqueResourceBlock createResourceBlock(std::vector<const Texture *> textures) override;
        virtual void destroyResourceBlock(UniqueResourceBlock &block) override;

        VulkanCommandList fetchCommandList(std::string_view name, std::vector<VkSemaphore> waitSemaphores = {}, std::vector<VkSemaphore> signalSemaphores = {});
        RenderFuture ExecuteCommandList(NulledPtr<std::remove_pointer_t<VkCommandBuffer>> buffer, std::vector<VkSemaphore> waitSemaphores, std::vector<VkSemaphore> signalSemaphores, std::vector<Any> attachedResources);

        VkDescriptorSet fetchTempDescriptorSet();

        bool isFenceComplete(uint64_t fenceValue);

        virtual Threading::Task<void> unloadAllResources() override;

        virtual bool supportsMultisampling() const override;

        static VulkanRenderContext &getSingleton();

        void createPipelineLayout();

        static std::pair<std::vector<VkVertexInputBindingDescription>, std::vector<VkVertexInputAttributeDescription>> createVertexLayout(VertexFormat format);

        size_t tempAllocatorMemoryQuota() const;

        VkQueue mGraphicsQueue;        

        QueueFamilyIndices mIndices;

        VulkanPtr<VkSemaphore, &vkDestroySemaphore> mSemaphore;
        uint64_t mLastCompletedFenceValue = 5;
        uint64_t mNextFenceValue;

        VulkanPtr<VkDescriptorPool, &vkDestroyDescriptorPool> mDescriptorPool;
        VulkanPtr<VkCommandPool, &vkDestroyCommandPool> mCommandPool;

        std::vector<std::tuple<uint64_t, VkCommandBuffer, std::vector<Any>>> mBufferPool;
        std::vector<std::tuple<uint64_t, VkDescriptorSet>> mTempDescriptorPool;

        VulkanPtr<VkDescriptorSetLayout, &vkDestroyDescriptorSetLayout> mUBODescriptorSetLayout;
        VulkanPtr<VkDescriptorSetLayout, &vkDestroyDescriptorSetLayout> mHeapDescriptorSetLayout;
        VulkanPtr<VkDescriptorSetLayout, &vkDestroyDescriptorSetLayout> mTempBufferDescriptorSetLayout;
        VulkanPtr<VkDescriptorSetLayout, &vkDestroyDescriptorSetLayout> mResourceBlockDescriptorSetLayout;
        VulkanPtr<VkDescriptorSetLayout, &vkDestroyDescriptorSetLayout> mSamplerDescriptorSetLayout;
        VulkanPtr<VkPipelineLayout, &vkDestroyPipelineLayout> mPipelineLayout;

        VkDescriptorSet mDefaultResourceBlockDescriptorSet;

        VulkanPtr<VkSampler, &vkDestroySampler> mSamplers[2];
        VkDescriptorSet mSamplerDescriptorSet;

        VulkanMappedHeapAllocator mUploadHeap;
        BucketAllocator<HeapAllocator<VulkanMappedHeapAllocator &>, 16, 64, 16> mUploadAllocator;

        VulkanHeapAllocator mBufferMemoryHeap;
        LogBucketAllocator<HeapAllocator<VulkanHeapAllocator &>, 64, 4096, 4> mBufferAllocator;

        VulkanMappedHeapAllocator mTempMemoryHeap;
        TrackedAllocator<BumpAllocator<FixedAllocator<VulkanMappedHeapAllocator &>>> mTempAllocator;

        VulkanHeapAllocator mConstantMemoryHeap;
        LogBucketAllocator<HeapAllocator<VulkanHeapAllocator &>, 64, 4096, 4> mConstantAllocator;

        VulkanBuffer mConstantBuffer;
        VulkanTexture mDefaultTexture;
    };

    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);

}
}

REGISTER_TYPE(Engine::Render::VulkanRenderContext)