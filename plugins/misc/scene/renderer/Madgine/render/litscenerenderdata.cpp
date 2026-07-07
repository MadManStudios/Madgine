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

        co_await mScene.mutex().locked(AccessMode::READ, [this]() {
            // TODO Culling

            for (auto &[mesh, entity] : mScene.entityComponentList<Scene::Entity::Mesh>().data()) {               

                if (!mesh.mIsVisible)
                    continue;

                const GPUMeshData *meshData = mesh.data();
                if (!meshData)
                    continue;

                Scene::Entity::Transform *transform = entity.getComponent<Scene::Entity::Transform>();
                if (!transform)
                    continue;

                const GPUMeshData::Material *material = nullptr;
                Scene::Entity::Material *materialComponent = entity.getComponent<Scene::Entity::Material>();
                if (materialComponent) {
                    material = &materialComponent->mMaterial;
                } else if (mesh.mMaterial < meshData->mMaterials.size()) {
                    material = &meshData->mMaterials[mesh.mMaterial];
                }
                ResourceBlock resource;
                Math::Vector4 diffuseColor { 1.0f, 1.0f, 1.0f, 1.0f };
                if (material) {
                    resource = material->mResourceBlock;
                    diffuseColor = material->mDiffuseColor;
                }

                Scene::Entity::Skeleton *skeleton = entity.getComponent<Scene::Entity::Skeleton>();
                Engine::Render::GPUPtr<Math::Matrix4[]> bones;
                if (skeleton)
                    bones = skeleton->mBoneMatrices;

                mInstances[NonInstancedData { meshData, resource }].push_back({ transform->worldMatrix(entity), diffuseColor, bones });
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
