#pragma once

#include "Madgine_Tools/toolbase.h"
#include "Madgine_Tools/toolscollector.h"

#include "OpenGL/util/opengltexture.h"
#include "Madgine_Tools/render/rendercontexttool.h"

namespace Engine {
namespace Tools {

    struct OpenGLRenderContextTool : public ToolVirtualImpl<OpenGLRenderContextTool, RenderContextTool> {

        SERIALIZABLEUNIT(OpenGLRenderContextTool)

        OpenGLRenderContextTool(ImRoot &root);

        virtual Threading::Task<bool> init() override;
        virtual Threading::Task<void> finalize() override;

        std::string_view key() const override;

    private:
        Render::OpenGLTexture mImageTexture;
    };

}
}

REGISTER_TYPE(Engine::Tools::OpenGLRenderContextTool)