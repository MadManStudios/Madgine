#include "../opengllib.h"

#include "openglshader.h"

namespace Engine {
namespace Render {

    GLenum toGLType(ShaderType type)
    {
        switch (type) {
        case PixelShader:
            return GL_FRAGMENT_SHADER;
        case VertexShader:
            return GL_VERTEX_SHADER;
        default:
            std::terminate();
        }
    }

    OpenGLShader::OpenGLShader(ShaderType type)
        : mHandle(glCreateShader(toGLType(type)))
        , mType(type)
    {
    }

    OpenGLShader::OpenGLShader(OpenGLShader &&other)
        : mHandle(std::exchange(other.mHandle, 0))
        , mType(std::exchange(other.mType, ShaderType {}))
    {
    }

    OpenGLShader::~OpenGLShader()
    {
        reset();
    }

    OpenGLShader &OpenGLShader::operator=(OpenGLShader &&other)
    {
        std::swap(mHandle, other.mHandle);
        std::swap(mType, other.mType);
        return *this;
    }

    void OpenGLShader::reset()
    {
        if (mHandle) {
            glDeleteShader(mHandle);
            mHandle = 0;
        }
    }

}
}