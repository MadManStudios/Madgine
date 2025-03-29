#include "../opengllib.h"

#include "openglpipeline.h"
#include "openglshader.h"

#include "Meta/math/matrix4.h"

#include "Meta/keyvalue/metatable_impl.h"

#include "Generic/bytebuffer.h"

#include "openglvertexarray.h"

METATABLE_BEGIN(Engine::Render::OpenGLPipeline)
METATABLE_END(Engine::Render::OpenGLPipeline)

namespace Engine {
namespace Render {

    OpenGLPipeline::~OpenGLPipeline()
    {
        reset();
    }

    OpenGLPipeline::OpenGLPipeline(OpenGLPipeline &&other)
        : mHandle(std::exchange(other.mHandle, 0))
    {
    }

    OpenGLPipeline &OpenGLPipeline::operator=(OpenGLPipeline &&other)
    {
        std::swap(mHandle, other.mHandle);
        return *this;
    }

    bool OpenGLPipeline::link(GLuint vertexShader, GLuint pixelShader)
    {
        reset();

        mHandle = glCreateProgram();
        glAttachShader(mHandle, vertexShader);
        if (pixelShader)
            glAttachShader(mHandle, pixelShader);

        glLinkProgram(mHandle);
        // check for linking errors
        GLint success;
        glGetProgramiv(mHandle, GL_LINK_STATUS, &success);
        if (!success) {
            char infoLog[8192];
            glGetProgramInfoLog(mHandle, 8192, NULL, infoLog);
            LOG_ERROR("ERROR::SHADER::PROGRAM::LINKING_FAILED");
            LOG_ERROR(infoLog);
            return false;
        }

#if OPENGL_ES
        GLuint perApplicationIndex = glGetUniformBlockIndex(mHandle, "PerApplication");
        if (perApplicationIndex != GL_INVALID_INDEX) {
            glUniformBlockBinding(mHandle, perApplicationIndex, 0);
            GL_CHECK();
        }

        GLuint perFrameIndex = glGetUniformBlockIndex(mHandle, "PerFrame");
        if (perFrameIndex != GL_INVALID_INDEX) {
            glUniformBlockBinding(mHandle, perFrameIndex, 1);
            GL_CHECK();
        }

        GLuint perObjectIndex = glGetUniformBlockIndex(mHandle, "PerObject");
        if (perObjectIndex != GL_INVALID_INDEX) {
            glUniformBlockBinding(mHandle, perObjectIndex, 2);
            GL_CHECK();
        }

        GLuint SSBOIndex = glGetUniformBlockIndex(mHandle, "SSBO");
        if (SSBOIndex != GL_INVALID_INDEX) {
            glUniformBlockBinding(mHandle, SSBOIndex, 3);
            GL_CHECK();
        }

        GLuint SSBOOffsetsIndex = glGetUniformBlockIndex(mHandle, "SSBO_Info");
        if (SSBOOffsetsIndex != GL_INVALID_INDEX) {
            glUniformBlockBinding(mHandle, SSBOOffsetsIndex, 4);
            GL_CHECK();
        }

#endif

        return true;
    }

    bool OpenGLPipeline::link(typename OpenGLShaderLoader::Handle vertexShader, typename OpenGLShaderLoader::Handle pixelShader)
    {
        if (!vertexShader || vertexShader->mType != VertexShader || (pixelShader && pixelShader->mType != PixelShader))
            std::terminate();

        return link(vertexShader ? vertexShader->mHandle : 0, pixelShader ? pixelShader->mHandle : 0);
    }

    bool OpenGLPipeline::link(typename OpenGLShaderLoader::Ptr vertexShader, typename OpenGLShaderLoader::Ptr pixelShader)
    {
        if (!vertexShader || vertexShader->mType != VertexShader || (pixelShader && pixelShader->mType != PixelShader))
            std::terminate();

        return link(vertexShader ? vertexShader->mHandle : 0, pixelShader ? pixelShader->mHandle : 0);
    }

    void OpenGLPipeline::reset()
    {
        if (mHandle) {
            glDeleteProgram(mHandle);
            mHandle = 0;
        }
    }

    void OpenGLPipeline::bind() const
    {
        glUseProgram(mHandle);
        GL_CHECK();
    }

    GLuint OpenGLPipeline::handle() const
    {
        return mHandle;
    }

}
}