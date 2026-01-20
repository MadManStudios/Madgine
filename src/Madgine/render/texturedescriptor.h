#pragma once

namespace Engine {
namespace Render {

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