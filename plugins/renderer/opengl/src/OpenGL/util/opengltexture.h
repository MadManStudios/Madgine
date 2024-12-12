#pragma once

#include "Meta/math/vector2i.h"

#include "Madgine/render/texture.h"

#include "Generic/bytebuffer.h"

#include "Madgine/render/texturedescriptor.h"

namespace Engine {
namespace Render {

    template <size_t I = 1>
    struct OpenGLResourceBlock {
        size_t mSize = I;
        struct {
            GLenum mTarget;
            GLint mHandle;
        } mResources[I];
    };

    struct MADGINE_OPENGL_EXPORT OpenGLTexture : Texture {

        OpenGLTexture(TextureType type, TextureFormat format, Vector2i size, size_t samples = 1, const ByteBuffer &data = {});
        OpenGLTexture(TextureType type, TextureFormat format, size_t samples = 1);
        OpenGLTexture() = default;
        OpenGLTexture(const OpenGLTexture &) = delete;
        OpenGLTexture(OpenGLTexture &&);
        ~OpenGLTexture();

        OpenGLTexture &operator=(OpenGLTexture &&);

        void reset();
        void bind() const;

        void setData(Vector2i size, const ByteBuffer &data);
        void setSubData(Vector2i offset, Vector2i size, const ByteBuffer &data);

        GLenum target() const;

        void setWrapMode(GLint mode);

        void setFilter(GLint filter);

    private:
        size_t mSamples = 1;

        OpenGLResourceBlock<1> mBlock;
    };

}
}