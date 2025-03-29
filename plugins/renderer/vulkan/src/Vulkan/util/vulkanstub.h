#pragma once

#include "Generic/fixed_string.h"

namespace Engine {
namespace Render {

    extern VkDevice GetDevice();
    extern VkInstance GetInstance();

    template <fixed_string Name, typename R, typename... Args>
    R vulkanDeviceStub(Args... args)
    {
        static R (*impl)(Args...) = (R(*)(Args...))vkGetDeviceProcAddr(GetDevice(), Name.c_str());
        return impl(args...);
    }

    template <fixed_string Name, typename R, typename... Args>
    R vulkanInstanceStub(Args... args)
    {
        static R (*impl)(Args...) = (R(*)(Args...))vkGetInstanceProcAddr(GetInstance(), Name.c_str());
        return impl(args...);
    }

    template <fixed_string Name, typename R, typename... Args>
    R vulkanInstanceStub(VkInstance instance, Args... args)
    {
        static R (*impl)(VkInstance, Args...) = (R(*)(VkInstance, Args...))vkGetInstanceProcAddr(instance, Name.c_str());
        return impl(instance, args...);
    }

}
}
