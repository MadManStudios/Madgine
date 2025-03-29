#include "opengllib.h"

#include "opengltextureloader.h"

#include "util/opengltexture.h"

#include "Meta/keyvalue/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

#include "openglrendercontext.h"

RESOURCELOADER(Engine::Render::OpenGLTextureLoader);

namespace Engine {
namespace Render {

    OpenGLTextureLoader::OpenGLTextureLoader()
    {
    }

    bool OpenGLTextureLoader::loadImpl(OpenGLTexture &tex, ResourceDataInfo &info)
    {
        throw 0;
    }

    void OpenGLTextureLoader::unloadImpl(OpenGLTexture &tex)
    {
        tex.reset();
    }

    bool OpenGLTextureLoader::create(Texture &tex, TextureType type, TextureFormat format)
    {
        static_cast<OpenGLTexture &>(tex) = OpenGLTexture { type, format };

        return true;
    }

    void OpenGLTextureLoader::setData(Texture &tex, Vector2i size, const ByteBuffer &data)
    {
        static_cast<OpenGLTexture &>(tex).setData(size, data);
    }

    void OpenGLTextureLoader::setSubData(Texture &tex, Vector2i offset, Vector2i size, const ByteBuffer &data)
    {
        static_cast<OpenGLTexture &>(tex).setSubData(offset, size, data);
    }

    Threading::TaskQueue *OpenGLTextureLoader::loadingTaskQueue() const
    {
        return OpenGLRenderContext::renderQueue();
    }

}
}
