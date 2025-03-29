#pragma once

#include "Madgine/render/renderdata.h"

#include "Madgine/meshloader/gpumeshdata.h"

#include "Madgine/render/ptr.h"

namespace Engine {
namespace Render {

    struct ShadowSceneRenderData : RenderData {

        ShadowSceneRenderData(SceneMainWindowComponent &scene, Camera *camera);

        virtual Threading::ImmediateTask<RenderFuture> render(RenderContext *context) override;

        SceneMainWindowComponent &mScene;

        Camera *mCamera;

        struct ObjectData {
            Matrix4 mTransform;
            GPUPtr<Matrix4[]> mBones;
        };
        std::map<const GPUMeshData *, std::vector<ObjectData>> mInstances;
    };

}
}