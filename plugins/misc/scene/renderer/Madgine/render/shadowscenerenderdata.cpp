#include "../scenerendererlib.h"

#include "shadowscenerenderdata.h"

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

    ShadowSceneRenderData::ShadowSceneRenderData(Scene::SceneManager &scene, SceneRenderData &renderData)
        : mScene(scene)
        , mRenderData(renderData)
    {
    }

    Threading::ImmediateTask<RenderFuture> ShadowSceneRenderData::render(RenderContext *context)
    {
        co_await mRenderData.update(context);

        for (auto &[key, transforms] : mInstances)
            transforms.clear();

        // Culled per view by the passes (FrustumCuller)

        for (const EntityRenderData &entity : mRenderData.data()) {

            if (!entity.mMesh || !entity.mCastShadows)
                continue;

            mInstances[entity.mMesh].push_back({ entity.mTransformMatrix, entity.mBones });
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
