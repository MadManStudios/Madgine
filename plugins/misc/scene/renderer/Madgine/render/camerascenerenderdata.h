#pragma once

#include "Madgine/meshloader/gpumeshdata.h"
#include "Madgine/meshloader/gpumeshloader.h"
#include "Madgine/render/ptr.h"
#include "Madgine/render/renderdata.h"

namespace Engine {
namespace Render {

    struct CameraSceneRenderData : RenderData {

        CameraSceneRenderData(SceneRenderData &renderData, Camera &camera);
        ~CameraSceneRenderData();

        Threading::ImmediateTask<RenderFuture> render(RenderContext *context) override;

        void unregister();

        Camera &mCamera;

        float mMaxAspectRatio = 0.0f;
        size_t mRefCount = 0;

        SceneRenderData &mRenderData;

        struct NonInstancedData {
            GPUMeshLoader::Handle mMesh;
            ResourceBlock mMaterial;
            bool mTransparentContent;

            constexpr auto operator<=>(const NonInstancedData &) const = default;
        };

        const std::map<NonInstancedData, std::vector<const EntityRenderData *>> &get(float aspectRatio);

        const Math::Matrix4 &getViewMatrix() const;

    private:
        Math::Matrix4 mViewMatrix;

        std::map<NonInstancedData, std::vector<const EntityRenderData *>> mInstances;
    };

}
}