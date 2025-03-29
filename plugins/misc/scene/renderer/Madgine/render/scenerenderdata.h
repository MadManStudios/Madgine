#pragma once

#include "Madgine/render/renderdata.h"

namespace Engine {
namespace Render {

    struct SceneRenderData : RenderData {

        SceneRenderData(Scene::SceneManager &scene);

        virtual Threading::ImmediateTask<RenderFuture> render(RenderContext *context) override;

    private:
        Scene::SceneManager &mScene;
    };

}
}