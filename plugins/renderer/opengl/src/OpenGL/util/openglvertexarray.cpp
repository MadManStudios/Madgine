#include "../opengllib.h"

#include "openglvertexarray.h"

namespace Engine {
namespace Render {

    OpenGLVertexArray::OpenGLVertexArray(create_t)
    {
        glGenVertexArrays(1, &mHandle);
    }

    OpenGLVertexArray::OpenGLVertexArray(OpenGLVertexArray &&other)
        : mHandle(std::exchange(other.mHandle, 0))
    {
    }

    OpenGLVertexArray::~OpenGLVertexArray()
    {
        reset();
    }

    OpenGLVertexArray &OpenGLVertexArray::operator=(OpenGLVertexArray &&other)
    {
        std::swap(mHandle, other.mHandle);
        return *this;
    }

    OpenGLVertexArray::operator bool() const
    {
        return mHandle != 0;
    }

    void OpenGLVertexArray::reset()
    {
        if (mHandle) {
            glDeleteVertexArrays(1, &mHandle);
            GL_CHECK();
            mHandle = 0;
        }
    }

    void OpenGLVertexArray::bind() const
    {
        glBindVertexArray(mHandle);
        GL_CHECK();
    }

    void OpenGLVertexArray::unbind()
    {
        glBindVertexArray(0);
        GL_CHECK();
    }

    GLuint OpenGLVertexArray::handle()
    {
        return mHandle;
    }

}
}