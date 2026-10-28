#include "../scenerendererlib.h"

#include "litscenerenderdata.h"

#include "Madgine/render/rendercontext.h"
#include "Madgine/scene/entity/components/material.h"
#include "Madgine/scene/entity/components/mesh.h"
#include "Madgine/scene/entity/components/skeleton.h"
#include "Madgine/scene/entity/components/transform.h"
#include "Madgine/scene/scenemanager.h"
#include "Madgine/window/mainwindow.h"

#include "scenemainwindowcomponent.h"

namespace Engine {
namespace Render {

    LitSceneRenderData::LitSceneRenderData(Scene::SceneManager &scene, SceneRenderData &renderData, Camera &camera)
        : mScene(scene)
        , mCamera(camera)
        , mRenderData(renderData)
    {
    }

    Threading::ImmediateTask<RenderFuture> LitSceneRenderData::render(RenderContext *context)
    {

        co_await mRenderData.update(context);

        for (auto &[key, transforms] : mInstances)
            transforms.clear();

        // Culled per view by the passes (FrustumCuller)

        for (const EntityRenderData &entity : mRenderData.data()) {     
            
            if (!entity.mMesh)
                continue;

            mInstances[NonInstancedData { entity.mMesh, entity.mResource }].push_back({ entity.mTransformMatrix, entity.mDiffuseColor, entity.mEmissiveColor, entity.mBones });
        }

        for (auto it = mInstances.begin(); it != mInstances.end();) {
            if (it->second.empty()) {
                it = mInstances.erase(it);
            } else {
                ++it;
            }
        }

        co_return {};
    }

}
}
