#pragma once

#include "Madgine/render/renderdata.h"
#include "Madgine/render/resourceblock.h"

namespace Engine {
namespace Render {

    struct MADGINE_SCENE_RENDERER_EXPORT SceneRenderData : RenderData {

        SceneRenderData(Scene::SceneManager &scene);

        virtual Threading::ImmediateTask<RenderFuture> render(RenderContext *context) override;

        void setup(RenderContext *context);
        void shutdown(RenderContext *context);

        ResourceBlock defaultMaterial();

    private:
        Scene::SceneManager &mScene;

        UniqueResourceBlock mDefaultMaterial;
    };

}
}