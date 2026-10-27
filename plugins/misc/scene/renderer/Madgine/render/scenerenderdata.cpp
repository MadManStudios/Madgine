#include "../scenerendererlib.h"

#include "scenerenderdata.h"

#include "Generic/containers/safeiterator.h"

#include "Modules/threading/taskqueue.h"

#include "Madgine/render/rendercontext.h"
#include "Madgine/scene/behavior/animation.h"
#include "Madgine/scene/entity/components/material.h"
#include "Madgine/scene/entity/components/mesh.h"
#include "Madgine/scene/entity/components/skeleton.h"
#include "Madgine/scene/entity/components/transform.h"
#include "Madgine/scene/entity/entitycomponentlist.h"
#include "Madgine/scene/scenemanager.h"

#include "scene_hlsl.h"

namespace Engine {
namespace Render {

    SceneRenderData::SceneRenderData(Scene::SceneManager &scene)
        : mScene(scene)
    {
        mScene.taskQueue()->queue([this]() -> Threading::Task<void> {
            while (mScene.taskQueue()->running()) {
                co_await mScene.simulationClock().wait(0s);
                co_await mScene.mutex().locked(AccessMode::READ, [this]() {
                    auto &transforms = mScene.entityComponentList<Scene::Entity::Transform>().data();

                    if (mData.size() < transforms.nodes_size())
                        mData.resize(transforms.nodes_size());

                    auto it1 = transforms.nodes_begin();
                    auto it2 = mData.begin();
                    for (; it1 != transforms.nodes_end(); ++it1, ++it2) {

                        if (it1->unsafeAccess().isFree())
                            continue;

                        Scene::Entity::Entity &e = *it1->unsafeAccess().mEntity;
                        Scene::Entity::Transform &t = it1->unsafeAccess().mComponent;
                        EntityRenderData &d = *it2;

                        d.mPreviousTransform = t;
                        d.mLastUpdate = mScene.now();
                    }
                });
            }
        });
    }

    void SceneRenderData::interpolateTransform(EntityRenderData &d, Scene::Entity::Transform &t, Scene::Entity::Entity &e)
    {
        auto &transforms = mScene.entityComponentList<Scene::Entity::Transform>().data();

        size_t frame = this->frame();

        if (d.mLastRenderedFrame < frame) {
            d.ensureAlive(mScene);

            float alpha = Math::clamp(std::chrono::duration_cast<std::chrono::duration<float, std::milli>>(mScene.animationClock().lastTick() - d.mLastUpdate).count() / 30.0f, 0.0f, 1.0f);

            Scene::Entity::Transform interpolated;

            interpolated.mPosition = Math::lerp(d.mPreviousTransform.mPosition, t.mPosition, alpha);
            interpolated.mScale = t.mScale;
            interpolated.mOrientation = Math::slerp(d.mPreviousTransform.mOrientation, t.mOrientation, alpha);
            d.mTransformMatrix = interpolated.matrix();

            if (Scene::Entity::EntityPtr parent = e.parent()) {
                Execution::access_binding(parent, [&](Scene::Entity::Entity &parent) {
                    Scene::Entity::Transform *t = parent.getComponent<Scene::Entity::Transform>();
                    if (t) {
                        auto it = std::find_if(transforms.nodes_begin(), transforms.nodes_end(), [&](auto &node) { return &node.unsafeAccess().mComponent == t; });
                        if (it != transforms.nodes_end()) {
                            EntityRenderData &parentData = *std::next(mData.begin(), std::distance(transforms.nodes_begin(), it));
                            interpolateTransform(parentData, *t, parent);
                            d.mTransformMatrix = parentData.mTransformMatrix * d.mTransformMatrix;
                        }
                    }
                });
            }

            d.mLastRenderedFrame = frame;
        }
    }

    Threading::ImmediateTask<RenderFuture> SceneRenderData::render(RenderContext *context)
    {
        co_await mScene.updateFrame([context](Scene::Entity::Skeleton *skeleton) -> Memory::TypedByteBuffer<Math::Matrix4[]> {
            throw "TODO";
                /*if (!skeleton->mBoneMatrices) {
                    skeleton->mBoneMatrices = context->allocateBuffer<Math::Matrix4[]>(skeleton->data()->mBones.size());
                }
                return context->mapBuffer(skeleton->mBoneMatrices);*/ });

        co_await mScene.mutex().locked(AccessMode::READ, [this]() {
            auto &transforms = mScene.entityComponentList<Scene::Entity::Transform>().data();

            if (mData.size() < transforms.nodes_size())
                mData.resize(transforms.nodes_size());

            auto it1 = transforms.nodes_begin();
            auto it2 = mData.begin();
            for (; it1 != transforms.nodes_end(); ++it1, ++it2) {

                EntityRenderData &d = *it2;

                if (it1->unsafeAccess().isFree()) {
                    d.markDead();
                    continue;
                }

                Scene::Entity::Entity &e = *it1->unsafeAccess().mEntity;
                Scene::Entity::Transform &t = it1->unsafeAccess().mComponent;

                interpolateTransform(d, t, e);

                Scene::Entity::Mesh *m = e.getComponent<Scene::Entity::Mesh>();
                d.mMesh = m && m->mMesh.available() && m->worldVisible(e) ? m->mMesh : GPUMeshLoader::Handle {};

                Scene::Entity::Material *mat = e.getComponent<Scene::Entity::Material>();

                if (mat) {
                    d.mMaterial = &mat->mMaterial;
                } else if (d.mMesh && m->mMaterial < d.mMesh->mMaterials.size()) {
                    d.mMaterial = &d.mMesh->mMaterials[m->mMaterial];
                }

                // TODO
                // Scene::Entity::Skeleton *skeleton = e.getComponent<Scene::Entity::
            }
        });
        co_return {};
    }

    void SceneRenderData::setup(RenderContext *context)
    {
        HLSL::sceneResourceBlock0 block;
        block.material./* emplace_back().*/ shininess = 128.0f;
        mDefaultMaterial = block.toResourceBlock(context).first;
    }

    void SceneRenderData::shutdown(RenderContext *context)
    {
        mData.clear();

        context->destroyResourceBlock(mDefaultMaterial);
    }

    ResourceBlock SceneRenderData::defaultMaterial()
    {
        return mDefaultMaterial;
    }

    Threading::TaskQueue *SceneRenderData::queue(RenderContext *)
    {
        return mScene.taskQueue();
    }

    void EntityRenderData::ensureAlive(Scene::SceneManager &scene)
    {
        if (!alive())
            mLastUpdate = scene.animationClock().lastTick();
    }

    void EntityRenderData::markDead()
    {
        mLastUpdate = std::chrono::steady_clock::time_point::min();
    }

    bool EntityRenderData::alive() const
    {
        return mLastUpdate.revert() != std::chrono::steady_clock::time_point::min();
    }

}
}