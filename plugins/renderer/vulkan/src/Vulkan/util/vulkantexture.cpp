#include "../vulkanlib.h"

#include "vulkantexture.h"

#include "Generic/align.h"

#include "../vulkanrendercontext.h"

namespace Engine {
namespace Render {

    extern void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer *buffer, VkDeviceMemory *bufferMemory);
    extern uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

    void transitionImageLayout(VkCommandBuffer commandList, VkImage image, VkFormat format, VkImageAspectFlags aspectMask, VkImageLayout oldLayout, VkImageLayout newLayout)
    {
        VkImageMemoryBarrier barrier {};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = oldLayout;
        barrier.newLayout = newLayout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = image;
        barrier.subresourceRange.aspectMask = aspectMask;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;
        VkPipelineStageFlags sourceStage;
        VkPipelineStageFlags destinationStage;

        switch (oldLayout) {
        case VK_IMAGE_LAYOUT_UNDEFINED:
            barrier.srcAccessMask = 0;
            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            break;
        case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
            barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
            sourceStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            break;
        case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            break;
        case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            break;
        default:
            throw 0;
        }

        switch (newLayout) {
        case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            break;
        case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            break;
        case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            break;
        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
            barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
            break;
        default:
            throw 0;
        }
        vkCmdPipelineBarrier(
            commandList,
            sourceStage, destinationStage,
            0,
            0, nullptr,
            0, nullptr,
            1, &barrier);
    }

    VulkanTexture::VulkanTexture(TextureType type, bool isRenderTarget, TextureFormat format, Vector2i size, size_t samples, const ByteBuffer &data)
        : Texture(type, format, size)
        , mIsRenderTarget(isRenderTarget)
        , mSamples(samples)
    {
        VkFormat vFormat = this->format();
        size_t byteCount;
        switch (format) {
        case FORMAT_RGBA8:
        case FORMAT_RGBA8_SRGB:
        case FORMAT_D24:
            byteCount = 4;
            break;
        case FORMAT_RGBA16F:
            byteCount = 8;
            break;
        default:
            std::terminate();
        }

        VkDeviceSize imageSize = size.x * size.y * byteCount;

        assert(samples == 1 || type == TextureType_2DMultiSample);

        unsigned int aspectMask = format == FORMAT_D24 ? VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT : VK_IMAGE_ASPECT_COLOR_BIT;

        VkImageViewCreateInfo viewInfo {};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.format = vFormat;
        viewInfo.subresourceRange.aspectMask = format == FORMAT_D24 ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        VkImageCreateInfo imageInfo {};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.extent.width = size.x;
        imageInfo.extent.height = size.y;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.format = vFormat;
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        if (isRenderTarget)
            imageInfo.usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        if (format == FORMAT_D24)
            imageInfo.usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageInfo.samples = static_cast<VkSampleCountFlagBits>(samples);
        imageInfo.flags = 0; // Optional

        switch (type) {
        case TextureType_2D:
        case TextureType_2DMultiSample:
            imageInfo.imageType = VK_IMAGE_TYPE_2D;
            viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            break;
        case TextureType_Cube:
            imageInfo.imageType = VK_IMAGE_TYPE_2D;
            imageInfo.arrayLayers = 6;
            imageInfo.flags |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
            viewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
            viewInfo.subresourceRange.layerCount = 6;
            break;
        default:
            std::terminate();
        }

        VkResult result = vkCreateImage(GetDevice(), &imageInfo, nullptr, &mTextureHandle.setupAs<VkImage>());
        VK_CHECK(result);

        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(GetDevice(), image(), &memRequirements);

        VkMemoryAllocateInfo allocInfo {};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        result = vkAllocateMemory(GetDevice(), &allocInfo, nullptr, &mDeviceMemory);
        VK_CHECK(result);

        vkBindImageMemory(GetDevice(), image(), mDeviceMemory, 0);

        VulkanRenderContext &context = VulkanRenderContext::getSingleton();

        VkCommandBufferAllocateInfo cmdAllocInfo {};
        cmdAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        cmdAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        cmdAllocInfo.commandPool = context.mCommandPool;
        cmdAllocInfo.commandBufferCount = 1;

        VkCommandBuffer commandBuffer;
        vkAllocateCommandBuffers(GetDevice(), &cmdAllocInfo, &commandBuffer);

        VkCommandBufferBeginInfo beginInfo {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(commandBuffer, &beginInfo);

        VulkanPtr<VkDeviceMemory, &vkFreeMemory> stagingBufferMemory;
        VulkanPtr<VkBuffer, &vkDestroyBuffer> stagingBuffer;
        if (data.mData) {

            createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingBuffer, &stagingBufferMemory);

            void *ptr;
            vkMapMemory(GetDevice(), stagingBufferMemory, 0, imageSize, 0, &ptr);
            memcpy(ptr, data.mData, static_cast<size_t>(imageSize));
            vkUnmapMemory(GetDevice(), stagingBufferMemory);

            transitionImageLayout(commandBuffer, image(), vFormat, aspectMask, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

            VkBufferImageCopy region {};
            region.bufferOffset = 0;
            region.bufferRowLength = 0;
            region.bufferImageHeight = 0;

            region.imageSubresource.aspectMask = aspectMask;
            region.imageSubresource.mipLevel = 0;
            region.imageSubresource.baseArrayLayer = 0;
            region.imageSubresource.layerCount = 1;

            region.imageOffset = { 0, 0, 0 };
            region.imageExtent = {
                static_cast<uint32_t>(size.x),
                static_cast<uint32_t>(size.y),
                1
            };
            vkCmdCopyBufferToImage(
                commandBuffer,
                stagingBuffer,
                image(),
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                1,
                &region);

            transitionImageLayout(commandBuffer, image(), vFormat, aspectMask, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        } else {
            transitionImageLayout(commandBuffer, image(), vFormat, aspectMask, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        }
        vkEndCommandBuffer(commandBuffer);

        VkSubmitInfo submitInfo {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        vkQueueSubmit(context.mGraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(context.mGraphicsQueue);

        vkFreeCommandBuffers(GetDevice(), context.mCommandPool, 1, &commandBuffer);

        viewInfo.image = image();

        result = vkCreateImageView(GetDevice(), &viewInfo, nullptr, &mImageView);
        VK_CHECK(result);

        VkDescriptorSet descriptorSet;

        VkDescriptorSetAllocateInfo descAllocInfo {};
        descAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        descAllocInfo.descriptorPool = context.mDescriptorPool;
        descAllocInfo.descriptorSetCount = 1;
        descAllocInfo.pSetLayouts = &std::as_const(context.mResourceBlockDescriptorSetLayout);
        result = vkAllocateDescriptorSets(GetDevice(), &descAllocInfo, &descriptorSet);
        VK_CHECK(result);

        VkDebugUtilsObjectNameInfoEXT nameInfo {};
        nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
        nameInfo.objectType = VK_OBJECT_TYPE_DESCRIPTOR_SET;
        nameInfo.objectHandle = reinterpret_cast<uintptr_t>(descriptorSet);
        nameInfo.pObjectName = "Direct Constant Texture Set";
        result = vkSetDebugUtilsObjectNameEXT(GetDevice(), &nameInfo);
        VK_CHECK(result);

        VkDescriptorImageInfo imageDescriptorInfo {};
        imageDescriptorInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageDescriptorInfo.imageView = mImageView;
        imageDescriptorInfo.sampler = VK_NULL_HANDLE;

        VkWriteDescriptorSet descriptorWrite {};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = descriptorSet;
        descriptorWrite.dstBinding = 0;
        descriptorWrite.dstArrayElement = 0;

        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        descriptorWrite.descriptorCount = 1;

        descriptorWrite.pBufferInfo = nullptr;
        descriptorWrite.pImageInfo = &imageDescriptorInfo; // Optional
        descriptorWrite.pTexelBufferView = nullptr; // Optional

        vkUpdateDescriptorSets(GetDevice(), 1, &descriptorWrite, 0, nullptr);

        mResourceBlock.setupAs<VkDescriptorSet>() = descriptorSet;
    }

    VulkanTexture::VulkanTexture(TextureType type, bool isRenderTarget, TextureFormat format, size_t samples)
        : Texture(type, format)
        , mIsRenderTarget(isRenderTarget)
        , mSamples(samples)
    {
    }

    VulkanTexture::VulkanTexture(VulkanTexture &&other)
        : Texture(std::move(other))
        , mIsRenderTarget(std::exchange(other.mIsRenderTarget, false))
        , mSamples(std::exchange(other.mSamples, 1))
    {
    }

    VulkanTexture::~VulkanTexture()
    {
        reset();
    }

    VulkanTexture &VulkanTexture::operator=(VulkanTexture &&other)
    {
        Texture::operator=(std::move(other));
        std::swap(mDeviceMemory, other.mDeviceMemory);
        std::swap(mIsRenderTarget, other.mIsRenderTarget);
        std::swap(mImageView, other.mImageView);
        std::swap(mSamples, other.mSamples);
        return *this;
    }

    void VulkanTexture::reset()
    {
        if (mResourceBlock) {
            VkDescriptorSet set = mResourceBlock.release<VkDescriptorSet>();
            vkFreeDescriptorSets(GetDevice(), VulkanRenderContext::getSingleton().mDescriptorPool, 1, &set);
        }
        mImageView.reset();
        mDeviceMemory.reset();
        if (mTextureHandle) {
            vkDestroyImage(GetDevice(), mTextureHandle.release<VkImage>(), nullptr);
        }
        mSamples = 1;
    }

    void VulkanTexture::setData(Vector2i size, const ByteBuffer &data)
    {
        *this = VulkanTexture { mType, mIsRenderTarget, mFormat, size, mSamples, data };
    }

    void VulkanTexture::setSubData(Vector2i offset, Vector2i size, const ByteBuffer &data)
    {
        VkFormat vFormat = format();
        VkImageViewType vType;
        size_t byteCount;
        switch (mFormat) {
        case FORMAT_RGBA8:
        case FORMAT_RGBA8_SRGB:
            byteCount = 4;
            break;
        case FORMAT_RGBA16F:
            byteCount = 8;
            break;
        default:
            std::terminate();
        }
        VkDeviceSize imageSize = size.x * size.y * byteCount;

        VulkanRenderContext &context = VulkanRenderContext::getSingleton();

        VkCommandBufferAllocateInfo cmdAllocInfo {};
        cmdAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        cmdAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        cmdAllocInfo.commandPool = context.mCommandPool;
        cmdAllocInfo.commandBufferCount = 1;

        VkCommandBuffer commandBuffer;
        vkAllocateCommandBuffers(GetDevice(), &cmdAllocInfo, &commandBuffer);

        VkCommandBufferBeginInfo beginInfo {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(commandBuffer, &beginInfo);

        VulkanPtr<VkDeviceMemory, &vkFreeMemory> stagingBufferMemory;
        VulkanPtr<VkBuffer, &vkDestroyBuffer> stagingBuffer;
        if (data.mData) {

            createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingBuffer, &stagingBufferMemory);

            void *ptr;
            vkMapMemory(GetDevice(), stagingBufferMemory, 0, imageSize, 0, &ptr);
            memcpy(ptr, data.mData, static_cast<size_t>(imageSize));
            vkUnmapMemory(GetDevice(), stagingBufferMemory);

            transitionImageLayout(commandBuffer, image(), vFormat, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

            VkBufferImageCopy region {};
            region.bufferOffset = 0;
            region.bufferRowLength = 0;
            region.bufferImageHeight = 0;

            region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            region.imageSubresource.mipLevel = 0;
            region.imageSubresource.baseArrayLayer = 0;
            region.imageSubresource.layerCount = 1;

            region.imageOffset = { offset.x, offset.y, 0 };
            region.imageExtent = {
                static_cast<uint32_t>(size.x),
                static_cast<uint32_t>(size.y),
                1
            };
            vkCmdCopyBufferToImage(
                commandBuffer,
                stagingBuffer,
                image(),
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                1,
                &region);

            transitionImageLayout(commandBuffer, image(), vFormat, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        } else {
            transitionImageLayout(commandBuffer, image(), vFormat, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        }
        vkEndCommandBuffer(commandBuffer);

        VkSubmitInfo submitInfo {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        vkQueueSubmit(context.mGraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(context.mGraphicsQueue);

        vkFreeCommandBuffers(GetDevice(), context.mCommandPool, 1, &commandBuffer);
    }

    VkImageView VulkanTexture::view() const
    {
        return mImageView;
    }

    VkImage VulkanTexture::image() const
    {
        return mTextureHandle.as<VkImage>();
    }

    VkFormat VulkanTexture::format() const
    {
        switch (mFormat) {
        case FORMAT_RGBA8:
            return VK_FORMAT_R8G8B8A8_UNORM;
        case FORMAT_RGBA16F:
            return VK_FORMAT_R16G16B16A16_SFLOAT;
        case FORMAT_RGBA8_SRGB:
            return VK_FORMAT_R8G8B8A8_SRGB;
        case FORMAT_D24:
            return VK_FORMAT_D24_UNORM_S8_UINT;
        default:
            std::terminate();
        }
    }

    void VulkanTexture::setName(std::string_view name)
    {
    }

    void VulkanTexture::transition(VkCommandBuffer commandList, VkImageLayout oldLayout, VkImageLayout newLayout)
    {
        transitionImageLayout(commandList, image(), format(), VK_IMAGE_ASPECT_COLOR_BIT, oldLayout, newLayout);
    }
}
}