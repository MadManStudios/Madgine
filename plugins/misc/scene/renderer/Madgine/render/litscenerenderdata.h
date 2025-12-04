#pragma once

#include "Madgine/meshloader/gpumeshdata.h"
#include "Madgine/render/ptr.h"
#include "Madgine/render/renderdata.h"

namespace Engine {
namespace Render {

    struct LitSceneRenderData : RenderData {

        LitSceneRenderData(Scene::SceneManager &scene, SceneRenderData &renderData, Camera &camera);

        virtual Threading::ImmediateTask<RenderFuture> render(RenderContext *context) override;

        Scene::SceneManager &mScene;

        Camera &mCamera;

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

        SceneRenderData &mRenderData;
    };

}
}