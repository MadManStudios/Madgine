#pragma once

#include "Generic/functor.h"

namespace Engine {
namespace Render {

    MADGINE_VULKAN_EXPORT VkDevice GetDevice();
    MADGINE_VULKAN_EXPORT VkInstance GetInstance();

    template <typename T, auto D>
    void vulkanDeleter(T p)
    {
        (*D)(GetDevice(), p, nullptr);
    }

    template <typename T, auto D>
    void vulkanDeleter2(T p)
    {
        (*D)(p, nullptr);
    }

    template <typename T, auto D>
    void vulkanDeleter3(T p)
    {
        (*D)(GetInstance(), p, nullptr);
    }

    template <typename T, auto D, auto f = vulkanDeleter<T, D>>
    struct VulkanPtr : std::unique_ptr<std::remove_pointer_t<T>, Functor<f>> {

        T *operator&()
        {
            assert(!*this);
            return reinterpret_cast<T *>(this);
        }

        T const *operator&() const
        {
            return reinterpret_cast<T const *>(this);
        }

        operator T () const
        {
            return this->get();
        }
    };
        
    template <typename T, auto D>
    using VulkanPtr2 = VulkanPtr<T, D, vulkanDeleter2<T, D>>;

    template <typename T, auto D>
    using VulkanPtr3 = VulkanPtr<T, D, vulkanDeleter3<T, D>>;

}
}