#pragma once

#include "Meta/math/matrix4.h"

#include "vulkanrendertarget.h"

namespace Engine {
namespace Render {

    struct MADGINE_VULKAN_EXPORT VulkanRenderWindow : VulkanRenderTarget {
        VulkanRenderWindow(VulkanRenderContext *context, Platform::Window::OSWindow *w, size_t samples);
        VulkanRenderWindow(const VulkanRenderWindow &) = delete;
        ~VulkanRenderWindow();

        virtual bool skipFrame() override;
        virtual void beginFrame() override;
        virtual RenderFuture endFrame() override;

        virtual void beginIteration(size_t targetIndex, size_t targetCount, size_t targetSubresourceIndex) const override;
        virtual void endIteration(size_t targetIndex, size_t targetCount, size_t targetSubresourceIndex) const override;

        virtual bool resizeImpl(const Math::Vector2i &size) override;
        virtual Math::Vector2i size() const override;

        virtual Math::Matrix4 getClipSpaceMatrix() const override;
        virtual void setRenderSpace(const Math::Rect2i &space) override;
        virtual void setScissorsRect(const Math::Rect2i &space) override;

        uint32_t imageCount() const;
        uint32_t minImageCount() const;

        VkSurfaceKHR surface() const;

    protected:
        void create(const Math::Vector2i &size);
        void createSurface();

    private:
        Platform::Window::OSWindow *mWindow;

        uint64_t mResizeFence;
        Math::Vector2i mResizeTarget;

        Math::Matrix4 mClipSpaceRotation;

        VulkanPtr<VkSemaphore, &vkDestroySemaphore> mImageSemaphore;

        VkSurfaceCapabilitiesKHR mSurfaceCapabilities;
        VulkanPtr3<VkSurfaceKHR, &vkDestroySurfaceKHR> mSurface;
        VulkanPtr<VkSwapchainKHR, &vkDestroySwapchainKHR> mSwapChain;
        uint32_t mImageIndex;
        std::vector<VkImage> mSwapChainImages;
        std::vector<VulkanPtr<VkImageView, &vkDestroyImageView>> mSwapChainImageViews;
        std::vector<VulkanPtr<VkFramebuffer, &vkDestroyFramebuffer>> mFramebuffers;
        VkSurfaceFormatKHR mFormat;
    };

}
}