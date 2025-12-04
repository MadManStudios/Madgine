#pragma once

namespace Engine {
namespace Render {

    struct OpenGLShader {
        OpenGLShader() = default;
        OpenGLShader(ShaderType type);
        OpenGLShader(const OpenGLShader &) = delete;
        OpenGLShader(OpenGLShader &&);
        ~OpenGLShader();

        OpenGLShader &operator=(OpenGLShader &&);

        void reset();

        GLuint mHandle = 0;
        ShaderType mType;
    };

}
}