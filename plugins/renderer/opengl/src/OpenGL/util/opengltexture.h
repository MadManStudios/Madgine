#pragma once

#include "Generic/bytebuffer.h"

#include "Meta/math/vector2i.h"

#include "Madgine/render/texture.h"
#include "Madgine/render/texturedescriptor.h"

namespace Engine {
namespace Render {

    template <size_t I = 1>
    struct OpenGLResourceBlock {
        size_t mSize = I;
        std::variant<ConstTexturePtr, GPUPtr<void>, GPUPtr<Void[]>> mResources[I];
    };

    struct MADGINE_OPENGL_EXPORT OpenGLTexture : Texture {

        OpenGLTexture(TextureType type, TextureFormat format, Math::Vector2i size, size_t samples = 1, const Memory::ByteBuffer &data = {});
        OpenGLTexture(TextureType type, TextureFormat format, size_t samples = 1);
        OpenGLTexture() = default;
        OpenGLTexture(const OpenGLTexture &) = delete;
        OpenGLTexture(OpenGLTexture &&);
        ~OpenGLTexture();

        OpenGLTexture &operator=(OpenGLTexture &&);

        void reset();
        void bind() const;

        GLuint handle() const;

        void setSubData(Math::Vector2i offset, Math::Vector2i size, const Memory::ByteBuffer &data);

        GLenum target() const;

        void setWrapMode(GLint mode);

        void setFilter(GLint filter);

    private:
        size_t mSamples = 1;

        OpenGLResourceBlock<1> mBlock;

        GLuint mHandle = 0;
    };

}
}