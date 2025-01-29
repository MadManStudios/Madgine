#pragma once

#include "Madgine/render/renderdata.h"

#include "Madgine/meshloader/gpumeshdata.h"

#include "Madgine/render/ptr.h"

namespace Engine {
namespace Render {

    struct LitSceneRenderData : RenderData {

        LitSceneRenderData(SceneMainWindowComponent &scene, Camera *camera);

        virtual Threading::ImmediateTask<RenderFuture> render(RenderContext *context) override;

        SceneMainWindowComponent &mScene;

        Camera *mCamera;

        struct NonInstancedData {
            const GPUMeshData *mMesh;
            ResourceBlock mMaterial;

            constexpr auto operator<=>(const NonInstancedData &) const = default;
        };
        struct ObjectData {
            Matrix4 mTransform;
            Vector4 mDiffuseColor;
            GPUPtr<Matrix4[]> mBones;
        };
        std::map<NonInstancedData, std::vector<ObjectData>> mInstances;
    };

}
}