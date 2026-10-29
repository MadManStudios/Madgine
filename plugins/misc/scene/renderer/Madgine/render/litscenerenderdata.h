#pragma once

#include "Madgine/meshloader/gpumeshdata.h"
#include "Madgine/render/ptr.h"
#include "Madgine/render/renderdata.h"

#include "Madgine/meshloader/gpumeshloader.h"

namespace Engine {
namespace Render {

    struct LitSceneRenderData : RenderData {

        LitSceneRenderData(Scene::SceneManager &scene, SceneRenderData &renderData, Camera &camera);

        Threading::ImmediateTask<RenderFuture> render(RenderContext *context) override;

        Scene::SceneManager &mScene;

        Camera &mCamera;

        struct NonInstancedData {
            GPUMeshLoader::Handle mMesh;
            ResourceBlock mMaterial;
            bool mTransparentContent;

            constexpr auto operator<=>(const NonInstancedData &) const = default;
        };
        struct ObjectData {
            Math::Matrix4 mTransform;
            Math::Vector4 mDiffuseColor;
            Math::Vector3 mEmissiveColor;
            GPUPtr<Math::Matrix4[]> mBones;
        };
        std::map<NonInstancedData, std::vector<ObjectData>> mInstances;

        SceneRenderData &mRenderData;
    };

}
}