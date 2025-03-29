#pragma once

#include "Madgine/resources/virtualresourceloader.h"

#include "Modules/threading/workgroupstorage.h"

#include "Madgine/render/texture.h"

#include "Madgine/render/texturedescriptor.h"

#include "Generic/bytebuffer.h"

#include "Meta/math/vector2i.h"

namespace Engine {
namespace Render {

    enum WrapMode {
        WRAP_CLAMP_TO_EDGE		
    };

	enum MinMode {
		MIN_NEAREST
	};

    struct MADGINE_RENDER_EXPORT TextureLoader : Resources::VirtualResourceLoaderBase<TextureLoader, Texture, std::list<Placeholder<0>>, Threading::WorkGroupStorage> {

        using Base = Resources::VirtualResourceLoaderBase<TextureLoader, Texture, std::list<Placeholder<0>>, Threading::WorkGroupStorage>;

        struct MADGINE_RENDER_EXPORT Ptr : Base::Ptr {

            using Base::Ptr::Ptr;
            Ptr(Base::Ptr ptr)
                : Base::Ptr(std::move(ptr))
            {
            }

            Threading::TaskFuture<bool> create(TextureType type, TextureFormat format, Vector2i size = { 0, 0 }, ByteBuffer data = {}, TextureLoader *loader = &TextureLoader::getSingleton());
            Threading::Task<bool> createTask(TextureType type, TextureFormat format, Vector2i size = { 0, 0 }, ByteBuffer data = {}, TextureLoader *loader = &TextureLoader::getSingleton());

            void setData(Vector2i size, const ByteBuffer &data, TextureLoader *loader = &TextureLoader::getSingleton());
            void setSubData(Vector2i offset, Vector2i size, const ByteBuffer &data, TextureLoader *loader = &TextureLoader::getSingleton());
        };

        struct MADGINE_RENDER_EXPORT Handle : Base::Handle {

            using Base::Handle::Handle;
            Handle(Base::Handle handle)
                : Base::Handle(std::move(handle))
            {
            }

            Threading::TaskFuture<bool> loadFromImage(std::string_view name, TextureType type, TextureFormat format, TextureLoader *loader = &TextureLoader::getSingleton());
        };

        TextureLoader();

        virtual bool create(Texture &tex, TextureType type, TextureFormat format) = 0;

        virtual void setData(Texture &tex, Vector2i size, const ByteBuffer &data) = 0;
        virtual void setSubData(Texture &tex, Vector2i offset, Vector2i size, const ByteBuffer &data) = 0;
    };

}
}

REGISTER_TYPE(Engine::Render::TextureLoader)