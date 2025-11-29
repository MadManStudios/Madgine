#pragma once

#include "Madgine/render/renderdata.h"

namespace Engine {
namespace Render {

    struct MADGINE_SCENE_RENDERER_EXPORT SceneRenderData : RenderData {

        SceneRenderData(Scene::SceneManager &scene);

        virtual Threading::ImmediateTask<RenderFuture> render(RenderContext *context) override;

    private:
        Scene::SceneManager &mScene;
    };

}
}