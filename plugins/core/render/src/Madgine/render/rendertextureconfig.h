#pragma once

#include "Madgine/render/texturedescriptor.h"

namespace Engine {
namespace Render {

    struct RenderTextureConfig {
        std::string mName;
        
        TextureType mType = TextureType_2D;
        TextureFormat mFormat = FORMAT_RGBA8;

        bool mCreateDepthBufferView = false;

        size_t mSamples = 1;
        size_t mTextureCount = 1; 
        bool mFlipFlop = false;
        
        RenderTarget *mBlitSource = nullptr;
    };

}
}