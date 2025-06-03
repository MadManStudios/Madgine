#pragma once

#include "Madgine/render/renderpass.h"

#include "Madgine/render/pipelineloader.h"

#include "Madgine/meshloader/gpumeshloader.h"

namespace Engine {
namespace Render {

    struct MADGINE_CLIENT_TOOLS_EXPORT Im3DRenderPass : RenderPass {
        Im3DRenderPass(Camera *camera, int priority);

        void setup(RenderTarget *target) override;
        void render(RenderTarget *target, size_t iteration) override;

        int priority() const override;

        std::string_view name() const override;

    private:
        Camera *mCamera;

        int mPriority;
    };

}
}