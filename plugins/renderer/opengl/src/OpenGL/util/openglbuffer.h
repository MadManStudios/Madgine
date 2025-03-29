#pragma once

#include "Generic/bytebuffer.h"

namespace Engine {
namespace Render {

    struct MADGINE_OPENGL_EXPORT OpenGLBuffer {

        OpenGLBuffer(GLenum target, const ByteBuffer &data = {});
        OpenGLBuffer(const OpenGLBuffer &) = delete;
        OpenGLBuffer(OpenGLBuffer &&);
        ~OpenGLBuffer();

        OpenGLBuffer &operator=(const OpenGLBuffer &) = delete;
        OpenGLBuffer &operator=(OpenGLBuffer &&);

        explicit operator bool() const;

        void bind() const;
        void bindVertex(size_t stride) const;

        void reset();
        void setData(const ByteBuffer &data);
        void resize(size_t size);

        void setSubData(unsigned int offset, const ByteBuffer &data);

        GLuint handle() const;

    private:
        GLuint mHandle = 0;
        GLenum mTarget;
        size_t mSize = 0;
    };

}
}