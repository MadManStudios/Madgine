#pragma once

#include "Madgine/meshloader/gpumeshdata.h"
#include "Madgine/meshloader/gpumeshloader.h"
#include "Madgine/render/ptr.h"
#include "Madgine/render/renderdata.h"

namespace Engine {
namespace Render {

    struct MADGINE_SCENE_RENDERER_EXPORT ShadowSceneRenderData : RenderData {

        ShadowSceneRenderData(Scene::SceneManager &scene, SceneRenderData &renderData);

        virtual Threading::ImmediateTask<RenderFuture> render(RenderContext *context) override;

        Scene::SceneManager &mScene;

        struct ObjectData {
            Math::Matrix4 mTransform;
            GPUPtr<Math::Matrix4[]> mBones;
        };
        std::map<GPUMeshLoader::Handle, std::vector<ObjectData>> mInstances;

        SceneRenderData &mRenderData;

        float mMinAlpha = 0.0f;
    };

}
}