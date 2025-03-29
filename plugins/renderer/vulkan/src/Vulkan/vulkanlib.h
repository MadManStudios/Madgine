#pragma once

/// @cond

#include "Madgine/renderlib.h"
#include "Madgine/meshloaderlib.h"

#if defined(Vulkan_EXPORTS)
#    define MADGINE_VULKAN_EXPORT DLL_EXPORT
#else
#    define MADGINE_VULKAN_EXPORT DLL_IMPORT
#endif

#include "vulkanforward.h"

#define NOMINMAX
#if !ANDROID && !EMSCRIPTEN && !IOS
#    include "../glad/vulkan.h"
#elif IOS
#    include <OpenGLES/ES3/gl.h>
#elif EMSCRIPTEN
#    include <GLES3/gl3.h>
#else
#    include <vulkan/vulkan.h>

#    include "util/vulkanstub.h"

inline VkResult vkCreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkDebugUtilsMessengerEXT *pMessenger)
{
    return Engine::Render::vulkanInstanceStub<"vkCreateDebugUtilsMessengerEXT", VkResult>(instance, pCreateInfo, pAllocator, pMessenger);
}
inline void vkDestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks *pAllocator)
{
    return Engine::Render::vulkanInstanceStub<"vkDestroyDebugUtilsMessengerEXT", void>(instance, debugMessenger, pAllocator);
}
inline VkResult vkGetSemaphoreCounterValueKHR(VkDevice device, VkSemaphore semaphore, uint64_t *pValue)
{
    return Engine::Render::vulkanDeviceStub<"vkGetSemaphoreCounterValueKHR", VkResult>(device, semaphore, pValue);
}
inline void vkCmdBeginDebugUtilsLabelEXT(VkCommandBuffer commandBuffer, const VkDebugUtilsLabelEXT *pLabelInfo)
{
    return Engine::Render::vulkanInstanceStub<"vkCmdBeginDebugUtilsLabelEXT", void>(commandBuffer, pLabelInfo);
}
inline void vkCmdEndDebugUtilsLabelEXT(VkCommandBuffer commandBuffer)
{
    return Engine::Render::vulkanInstanceStub<"vkCmdEndDebugUtilsLabelEXT", void>(commandBuffer);
}
inline VkResult vkSetDebugUtilsObjectNameEXT(VkDevice device, const VkDebugUtilsObjectNameInfoEXT *pNameInfo)
{
    return Engine::Render::vulkanDeviceStub<"vkSetDebugUtilsObjectNameEXT", VkResult>(device, pNameInfo);
}
#endif

#include "util/vulkanptr.h"

inline void vkCheck(VkResult result)
{
    if (result != VK_SUCCESS) {
        LOG("Vulkan-Error: "
            << "?");
        std::terminate();
    }
}

#define VK_CHECK(...) vkCheck(__VA_ARGS__)

// #define VK_LOG(x) LOG("VK: " << x)
#define VK_LOG(x)

/// @endcond
