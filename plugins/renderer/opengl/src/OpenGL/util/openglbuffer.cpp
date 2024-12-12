#include "../opengllib.h"

#include "openglbuffer.h"

#if OPENGL_ES
#    include "openglvertexarray.h"
#endif

namespace Engine {
namespace Render {

    OpenGLBuffer::OpenGLBuffer(GLenum target, const ByteBuffer &data)
        : mTarget(target)
    {
        setData(data);
    }

    OpenGLBuffer::OpenGLBuffer(OpenGLBuffer &&other)
        : mHandle(std::exchange(other.mHandle, 0))
        , mTarget(std::exchange(other.mTarget, 0))
        , mSize(std::exchange(other.mSize, 0))
    {
    }

    OpenGLBuffer::~OpenGLBuffer()
    {
        reset();
    }

    OpenGLBuffer &OpenGLBuffer::operator=(OpenGLBuffer &&other)
    {
        std::swap(mHandle, other.mHandle);
        std::swap(mTarget, other.mTarget);
        std::swap(mSize, other.mSize);
        return *this;
    }

    OpenGLBuffer::operator bool() const
    {
        return mSize > 0;
    }

    void OpenGLBuffer::bind() const
    {
        glBindBuffer(mTarget, mHandle);
        GL_LOG("Bind Buffer " << mTarget << " -> " << mHandle);
        GL_CHECK();
    }

    void OpenGLBuffer::bindVertex(size_t stride) const
    {
        assert(mTarget == GL_ARRAY_BUFFER);

        bind();

#if !OPENGL_ES || OPENGL_ES >= 31
#    if !OPENGL_ES
        if (glBindVertexBuffer)
#    endif
            glBindVertexBuffer(0, mHandle, 0, stride);
#endif

        GL_CHECK();
    }

    void OpenGLBuffer::reset()
    {
        if (mHandle) {
            glDeleteBuffers(1, &mHandle);
            GL_CHECK();
            mHandle = 0;
            mTarget = 0;
        }
    }

    void OpenGLBuffer::setData(const ByteBuffer &data)
    {
        mSize = data.mSize;
        if (mSize > 0) {
            if (!mHandle) {
                glGenBuffers(1, &mHandle);
                GL_CHECK();
            }
            bind();
            glBufferData(mTarget, data.mSize, data.mData, data.mData ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW);
            GL_CHECK();
        }
    }

    void OpenGLBuffer::resize(size_t size)
    {
        if (size != mSize) {
            setData({ nullptr, size });
        }
    }

    void OpenGLBuffer::setSubData(unsigned int offset, const ByteBuffer &data)
    {
        bind();
        glBufferSubData(mTarget, offset, data.mSize, data.mData);
        GL_LOG("Buffer-SubData: " << offset << ", " << data.mSize);
        GL_CHECK();
    }

    GLuint OpenGLBuffer::handle() const
    {
        return mHandle;
    }

}
}