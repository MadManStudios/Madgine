#pragma once

#include "ptr.h"
#include "Generic/opaqueptr.h"

namespace Engine {
namespace Render {

    struct TextureHandle {

        template <typename T>
        operator T() const
        {
            return (T)(mPtr);
        }

        auto operator<=>(const TextureHandle &) const = default;

        uintptr_t mPtr;
    };

    struct UniqueTextureHandle : UniqueOpaquePtr {

        operator TextureHandle() const
        {
            return { get() };
        }
    };

    enum TextureType {
        TextureType_2D,
        TextureType_2DMultiSample,
        TextureType_Cube
    };

    enum TextureFormat {
        FORMAT_RGBA8,
        FORMAT_RGBA8_SRGB,
        FORMAT_RGBA16F,
        FORMAT_R32F,
        FORMAT_D24,
        FORMAT_D32
    };

}
}