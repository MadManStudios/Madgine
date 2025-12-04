#pragma once

#include "Madgine_Tools/rendertoolslib.h"
#include "Vulkan/vulkanlib.h"

#if defined(VulkanTools_EXPORTS)
#    define MADGINE_VULKAN_TOOLS_EXPORT DLL_EXPORT
#else
#    define MADGINE_VULKAN_TOOLS_EXPORT DLL_IMPORT
#endif

#include "vulkantoolsforward.h"