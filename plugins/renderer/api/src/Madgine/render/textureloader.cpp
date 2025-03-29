#include "../renderlib.h"

#include "textureloader.h"

#include "Meta/keyvalue/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

#include "Meta/math/vector2i.h"

#include "Madgine/imageloader/imageloader.h"

VIRTUALRESOURCELOADERBASE(Engine::Render::TextureLoader)

namespace Engine {
namespace Render {

    TextureLoader::TextureLoader()
        : VirtualResourceLoaderBase(std::vector<std::string> {})
    {
    }

    Threading::TaskFuture<bool> TextureLoader::Ptr::create(
        TextureType type, TextureFormat format, Vector2i size, ByteBuffer data, TextureLoader *loader)
    {
        return Base::Ptr::create(
            [=, data { std::move(data) }](TextureLoader *loader, Texture &texture) mutable {
                if (!loader->create(texture, type, format))
                    return false;
                if (data.mSize > 0)
                    loader->setData(texture, size, std::move(data));
                return true;
            },
            loader);        
    }

    Threading::Task<bool> TextureLoader::Ptr::createTask(
        TextureType type, TextureFormat format, Vector2i size, ByteBuffer data, TextureLoader *loader)
    {
        return Base::Ptr::createTask(
            [=, data { std::move(data) }](TextureLoader *loader, Texture &texture) mutable {
                if (!loader->create(texture, type, format))
                    return false;
                if (data.mSize > 0)
                    loader->setData(texture, size, std::move(data));
                return true;
            }, loader);
    }

    Threading::TaskFuture<bool> TextureLoader::Handle::loadFromImage(std::string_view name, TextureType type, TextureFormat format, TextureLoader *loader)
    {
        return Base::Handle::create(
            name, {}, [=](TextureLoader *loader, Texture &texture, const TextureLoader::ResourceDataInfo &info) -> Threading::Task<bool> {
                Resources::ImageLoader::Handle image;
                if (!co_await image.load(info.resource()->name())) {
                    LOG_ERROR("Unable to load image '" << info.resource()->name() << "'");
                    co_return false;
                }
                loader->create(texture, type, format);
                loader->setData(texture, image->mSize, image->mBuffer);
                co_return true;
            },
             loader);
            
    }

    void TextureLoader::Ptr::setData(Vector2i size, const ByteBuffer &data, TextureLoader *loader)
    {
        loader->setData(**this, size, data);
    }

    void TextureLoader::Ptr::setSubData(Vector2i offset, Vector2i size, const ByteBuffer &data, TextureLoader *loader)
    {
        loader->setSubData(**this, offset, size, data);
    }

}
}