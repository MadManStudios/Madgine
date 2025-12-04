#include "../scenerendererlib.h"

#include "shadowscenerenderdata.h"

#include "Modules/threading/awaitables/awaitablesender.h"

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

        co_await mScene.mutex().locked(AccessMode::READ, [this]() {
            // TODO Culling

            for (const auto &mesh : mScene.entityComponentList<Scene::Entity::Mesh>().data()) {
                if (!mesh.isVisible())
                    continue;

                const GPUMeshData *meshData = mesh.data();
                if (!meshData)
                    continue;

                Scene::Entity::Transform *transform = mesh.entity().getComponent<Scene::Entity::Transform>();
                if (!transform)
                    continue;

                Scene::Entity::Skeleton *skeleton = mesh.entity().getComponent<Scene::Entity::Skeleton>();
                Engine::Render::GPUPtr<Matrix4[]> bones;
                if (skeleton)
                    bones = skeleton->mBoneMatrices;

                mInstances[meshData].push_back({ transform->worldMatrix(), bones });
            }
        });

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
