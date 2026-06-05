#pragma once

#include "Madgine/resources/resourceloader.h"

#include "imagedata.h"

namespace Engine {
namespace Resources {

    struct MADGINE_IMAGELOADER_EXPORT ImageLoader : ResourceLoader<ImageLoader, ImageData> {

        ImageLoader();

        Threading::Task<bool> loadImpl(ImageData &data, ResourceDataInfo &info);
        Threading::Task<void> unloadImpl(ImageData &data);

        const Platform::Filesystem::Path &iconPath(ResourceBase *res) const override;

        static Memory::ByteBuffer convertFromPNG(const Memory::ByteBuffer &data, Math::Vector2i &outSize);
        static Memory::ByteBuffer convertToPNG(const Memory::ByteBuffer &data, Math::Vector2i size);
    };

}
}