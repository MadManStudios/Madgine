#pragma once

#include "Madgine/render/renderdata.h"

#include "pointshadowrenderpass.h"

namespace Engine {
namespace Render {

    struct MADGINE_SCENE_RENDERER_EXPORT PointShadowRenderData : RenderData {

        PointShadowRenderData(Scene::SceneManager &scene, SceneRenderData &sceneData);
        ~PointShadowRenderData();

        void setup(RenderContext *context);
        void shutdown(RenderContext *context);

        std::vector<const Texture *> depthTextures();

        Render::RenderTarget *pointShadowTarget(size_t index);

        Threading::ImmediateTask<RenderFuture> render(RenderContext *context) override;

    private:
        std::unique_ptr<Render::RenderTarget> mPointShadowMaps[2];

        PointShadowRenderPass mPointShadowPasses[2];
    };

}
}