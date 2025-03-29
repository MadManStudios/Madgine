#pragma once

#include "Madgine/render/pipeline.h"

#include "../openglshaderloader.h"

namespace Engine {
namespace Render {

    struct MADGINE_OPENGL_EXPORT OpenGLPipeline : Pipeline {

        OpenGLPipeline() = default;
        OpenGLPipeline(OpenGLPipeline &&other);
        ~OpenGLPipeline();

        OpenGLPipeline &operator=(OpenGLPipeline &&other);

        bool link(typename OpenGLShaderLoader::Handle vertexShader, typename OpenGLShaderLoader::Handle pixelShader = {});
        bool link(typename OpenGLShaderLoader::Ptr vertexShader, typename OpenGLShaderLoader::Ptr pixelShader = {});

        void reset();

        void bind() const;

        GLuint handle() const;

    protected:
        bool link(GLuint vertexShader, GLuint pixelShader);

    private:
        mutable GLuint mHandle = 0;
    };

}
}