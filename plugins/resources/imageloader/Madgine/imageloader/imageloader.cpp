#include "../imageloaderlib.h"

#include "imageloader.h"

#include "Meta/keyvalue/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

#include "Modules/threading/awaitables/awaitablesender.h"

#include "imagedata.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

RESOURCELOADER(Engine::Resources::ImageLoader)


namespace Engine {
namespace Resources {

    ImageLoader::ImageLoader()
        : ResourceLoader({ ".png", ".jpg" })
    {
    }

    Threading::Task<bool> ImageLoader::loadImpl(ImageData &data, ResourceDataInfo &info)
    {
        ByteBuffer buffer = (co_await info.resource()->readAsync()).value();

        data.mChannels = 4;

        data.mBuffer = convertFromPNG(buffer, data.mSize);

        co_return true;
    }

    Threading::Task<void> ImageLoader::unloadImpl(ImageData &data)
    {
        data.mBuffer.clear();
        co_return;
    }

    ByteBuffer ImageLoader::convertFromPNG(const ByteBuffer &data, Vector2i &outSize)
    {
        stbi_uc *ptr = stbi_load_from_memory(static_cast<const stbi_uc *>(data.mData), data.mSize,
            &outSize.x,
            &outSize.y,
            nullptr,
            STBI_rgb_alpha);

        return { std::unique_ptr<stbi_uc, Functor<&stbi_image_free>> { ptr }, static_cast<size_t>(outSize.x) * outSize.y * 4 };
    }

    ByteBuffer ImageLoader::convertToPNG(const ByteBuffer &data, Vector2i size)
    {
        ByteBuffer image;
        int result = stbi_write_png_to_func([](void *context, void *data, int size) {
            WritableByteBuffer output { std::make_unique<std::byte[]>(size), static_cast<size_t>(size) };
            std::memcpy(output.mData, data, size);
            *static_cast<ByteBuffer *>(context) = std::move(output).cast<const void>();
        },
            &image, size.x, size.y, STBI_rgb_alpha, data.mData, 4 * size.x);
        assert(result);
        return image;
    }

}
}