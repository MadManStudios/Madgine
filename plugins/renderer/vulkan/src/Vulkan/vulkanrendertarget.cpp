#include "vulkanlib.h"

#include "vulkanrendertarget.h"

#include "vulkanrendercontext.h"

#include "Meta/math/matrix4.h"
#include "Meta/math/rect2i.h"

namespace Engine {
namespace Render {

    extern uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
    extern void transitionImageLayout(VkCommandBuffer commandList, VkImage image, VkFormat format, VkImageAspectFlags aspectMask, VkImageLayout oldLayout, VkImageLayout newLayout);

    VulkanRenderTarget::VulkanRenderTarget(VulkanRenderContext *context, bool global, std::string name, TextureType type, size_t samples, RenderTarget *blitSource)
        : RenderTarget(context, global, name, 1, blitSource)
        , mDepthTexture(type, false, FORMAT_D24, samples)
        , mSamples(samples)
    {
        VkSemaphoreCreateInfo binarySemaphoreInfo {};
        binarySemaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        VkResult result = vkCreateSemaphore(GetDevice(), &binarySemaphoreInfo, nullptr, &mRenderSemaphore);
        VK_CHECK(result);
    }

    VulkanRenderTarget::~VulkanRenderTarget()
    {
    }

    void VulkanRenderTarget::createRenderPass(size_t colorAttachmentCount, VkFormat format, VkImageLayout layout, bool createDepthBufferView, std::span<VkSubpassDependency> dependencies)
    {

        std::vector<VkAttachmentDescription> attachments { 1 + colorAttachmentCount };
        std::vector<VkAttachmentReference> colorAttachmentRef { colorAttachmentCount };

        attachments[0].format = VK_FORMAT_D24_UNORM_S8_UINT;
        attachments[0].samples = static_cast<VkSampleCountFlagBits>(mSamples);

        attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachments[0].storeOp = createDepthBufferView ? VK_ATTACHMENT_STORE_OP_STORE : VK_ATTACHMENT_STORE_OP_DONT_CARE;

        attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

        attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachments[0].finalLayout = createDepthBufferView ? VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        for (size_t i = 0; i < colorAttachmentCount; ++i) {
            attachments[i + 1].format = format;
            attachments[i + 1].samples = static_cast<VkSampleCountFlagBits>(mSamples);

            attachments[i + 1].loadOp = mBlitSource ? VK_ATTACHMENT_LOAD_OP_LOAD : VK_ATTACHMENT_LOAD_OP_CLEAR;
            attachments[i + 1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;

            attachments[i + 1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            attachments[i + 1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

            attachments[i + 1].initialLayout = mBlitSource ? VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_UNDEFINED;
            attachments[i + 1].finalLayout = layout;

            colorAttachmentRef[i].attachment = i + 1;
            colorAttachmentRef[i].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        }

        VkAttachmentReference depthAttachmentRef {};
        depthAttachmentRef.attachment = 0;
        depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass {};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

        subpass.colorAttachmentCount = colorAttachmentCount;
        subpass.pColorAttachments = colorAttachmentRef.data();
        subpass.pDepthStencilAttachment = &depthAttachmentRef;

        VkRenderPassCreateInfo renderPassInfo {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = 1 + colorAttachmentCount;
        renderPassInfo.pAttachments = attachments.data();
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = dependencies.size();
        renderPassInfo.pDependencies = dependencies.data();

        mRenderPass.reset();
        VkResult result = vkCreateRenderPass(GetDevice(), &renderPassInfo, nullptr, &mRenderPass);
        VK_CHECK(result);
    }

    void VulkanRenderTarget::setup(const Vector2i &size, bool createDepthBufferView)
    {
        mSize = size;

        mDepthTexture.setData(size, {});

        VkImageCreateInfo imageInfo {};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = size.x;
        imageInfo.extent.height = size.y;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.format = VK_FORMAT_D24_UNORM_S8_UINT;
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        if (createDepthBufferView)
            imageInfo.usage |= VK_IMAGE_USAGE_SAMPLED_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageInfo.samples = static_cast<VkSampleCountFlagBits>(mSamples);
        imageInfo.flags = 0; // Optional

        VkImageViewCreateInfo viewInfo {};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = VK_FORMAT_D24_UNORM_S8_UINT;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;
    }

    void VulkanRenderTarget::shutdown()
    {
    }

    void VulkanRenderTarget::beginFrame()
    {
        VkRenderPassBeginInfo renderPassInfo {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = mRenderPass;
        renderPassInfo.framebuffer = mFramebuffer;

        renderPassInfo.renderArea.offset = { 0, 0 };
        renderPassInfo.renderArea.extent = { static_cast<uint32_t>(mSize.x), static_cast<uint32_t>(mSize.y) };

        VkClearValue clearColors[2] { { { 1.0f, 1.0f, 1.0f, 1.0f } }, { { 0.033f, 0.073f, 0.073f, 1.0f } } };
        renderPassInfo.clearValueCount = 2;
        renderPassInfo.pClearValues = clearColors;

        vkCmdBeginRenderPass(mCommandList, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        setRenderSpace({ { 0, 0 }, mSize });

        RenderTarget::beginFrame();
    }

    RenderFuture VulkanRenderTarget::endFrame()
    {
        RenderTarget::endFrame();

        vkCmdEndRenderPass(mCommandList);

        return mCommandList.reset();
    }

    Matrix4 VulkanRenderTarget::getClipSpaceMatrix() const
    {
        return Matrix4 { 1, 0, 0, 0,
            0, -1, 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1 };
    }

    void VulkanRenderTarget::beginIteration(size_t targetIndex, size_t targetCount, size_t targetSubresourceIndex) const
    {
        RenderTarget::beginIteration(targetIndex, targetCount, targetSubresourceIndex);
    }

    void VulkanRenderTarget::endIteration(size_t targetIndex, size_t targetCount, size_t targetSubresourceIndex) const
    {
        RenderTarget::endIteration(targetIndex, targetCount, targetSubresourceIndex);
    }

    void VulkanRenderTarget::setRenderSpace(const Rect2i &space)
    {
        VkViewport viewport {};
        viewport.width = static_cast<float>(space.mSize.x);
        viewport.height = static_cast<float>(space.mSize.y);
        viewport.x = static_cast<float>(space.mTopLeft.x);
        viewport.y = static_cast<float>(space.mTopLeft.y);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(mCommandList, 0, 1, &viewport);

        VkRect2D scissor {};
        scissor.offset = { space.mTopLeft.x, space.mTopLeft.y };
        scissor.extent = { static_cast<uint32_t>(space.mSize.x), static_cast<uint32_t>(space.mSize.y) };
        vkCmdSetScissor(mCommandList, 0, 1, &scissor);
    }

    void VulkanRenderTarget::setScissorsRect(const Rect2i &space)
    {
        VkRect2D scissor {};
        scissor.offset = { space.mTopLeft.x, space.mTopLeft.y };
        scissor.extent = { static_cast<uint32_t>(space.mSize.x), static_cast<uint32_t>(space.mSize.y) };
        vkCmdSetScissor(mCommandList, 0, 1, &scissor);
    }

    void VulkanRenderTarget::pushAnnotation(const char *tag)
    {
        VkDebugUtilsLabelEXT labelInfo {};
        labelInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
        std::fill_n(labelInfo.color, 4, 1.0f);
        labelInfo.pLabelName = tag;
        vkCmdBeginDebugUtilsLabelEXT(mCommandList, &labelInfo);
    }

    void VulkanRenderTarget::popAnnotation()
    {
        vkCmdEndDebugUtilsLabelEXT(mCommandList);
    }

    void VulkanRenderTarget::clearDepthBuffer()
    {
        VkClearAttachment depthAttachment {};
        depthAttachment.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        depthAttachment.clearValue.depthStencil.depth = 1.0f;
        VkClearRect r {};
        r.baseArrayLayer = 0;
        r.layerCount = 1;
        r.rect.extent = { (uint32_t)size().x, (uint32_t)size().y };
        r.rect.offset = { 0, 0 };

        vkCmdClearAttachments(mCommandList, 1, &depthAttachment, 1, &r);
    }

    const Texture *VulkanRenderTarget::depthTexture() const
    {
        return &mDepthTexture;
    }

    VulkanRenderContext *VulkanRenderTarget::context() const
    {
        return static_cast<VulkanRenderContext *>(RenderTarget::context());
    }

    size_t VulkanRenderTarget::samples() const
    {
        return mSamples;
    }

}
}
