#pragma once

#include "Madgine/meshloader/gpumeshloader.h"
#include "Madgine/render/pipelineloader.h"
#include "Madgine/render/renderpass.h"

namespace Engine {
namespace Im3D {
    struct Im3DContext;
}

namespace Render {

    struct MADGINE_CLIENT_TOOLS_EXPORT Im3DRenderPass : RenderPass {
        Im3DRenderPass(Im3D::Im3DContext *context, Camera *camera, int priority);

        void setup(RenderTarget *target) override;
        void shutdown(RenderTarget *target) override;
        void render(RenderTarget *target, size_t iteration) override;

        int priority() const override;

        std::string_view name() const override;

    private:
        Im3D::Im3DContext *mContext;

        Camera *mCamera;

        int mPriority;

        UniqueResourceBlock mDefaultBlock;
    };

}
}