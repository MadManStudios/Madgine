#pragma once

#include "Madgine/resources/resourceloader.h"

#include "Madgine/render/textureloader.h"

#include "util/opengltexture.h"

namespace Engine {
namespace Render {

    struct MADGINE_OPENGL_EXPORT OpenGLTextureLoader : Resources::VirtualResourceLoaderImpl<OpenGLTextureLoader, OpenGLTexture, TextureLoader> {
        OpenGLTextureLoader();

        bool loadImpl(OpenGLTexture &tex, ResourceDataInfo &info);
        void unloadImpl(OpenGLTexture &tex);
        bool create(Texture &texture, TextureType type, TextureFormat format) override;

		virtual void setData(Texture &tex, Vector2i size, const ByteBuffer &data) override;
        virtual void setSubData(Texture &tex, Vector2i offset, Vector2i size, const ByteBuffer &data) override;

         virtual Threading::TaskQueue *loadingTaskQueue() const override;
	};
}
}

REGISTER_TYPE(Engine::Render::OpenGLTextureLoader)