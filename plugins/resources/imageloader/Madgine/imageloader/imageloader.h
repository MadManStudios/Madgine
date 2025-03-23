#pragma once

#include "Madgine/resources/resourceloader.h"

#include "imagedata.h"

namespace Engine {
namespace Resources {

    struct MADGINE_IMAGELOADER_EXPORT ImageLoader : ResourceLoader<ImageLoader, ImageData> {

        ImageLoader();

        Threading::Task<bool> loadImpl(ImageData &data, ResourceDataInfo &info);
        Threading::Task<void> unloadImpl(ImageData &data);

        static ByteBuffer convertFromPNG(const ByteBuffer &data, Vector2i &outSize);
        static ByteBuffer convertToPNG(const ByteBuffer &data, Vector2i size);
    };

}
}

REGISTER_TYPE(Engine::Resources::ImageLoader)