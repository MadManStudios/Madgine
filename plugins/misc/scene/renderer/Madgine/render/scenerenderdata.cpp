#include "../scenerendererlib.h"

#include "scenerenderdata.h"

#include "Madgine/scene/scenemanager.h"

#include "Modules/threading/awaitables/awaitablesender.h"

#include "Madgine/scene/entity/entitycomponentlist.h"

#include "Madgine/scene/entity/components/skeleton.h"

#include "Madgine/render/rendercontext.h"

#include "Madgine/scene/behavior/animation.h"

#include "Madgine/scene/entity/components/transform.h"

#include "Generic/container/safeiterator.h"

#include "im3d/im3d.h"

namespace Engine {
namespace Render {

    SceneRenderData::SceneRenderData(Scene::SceneManager &scene)
        : mScene(scene)
    {
    }

    Threading::ImmediateTask<RenderFuture> SceneRenderData::render(RenderContext *context)
    {
        co_await mScene.mutex().locked(Engine::AccessMode::READ, [this, context]() {
            mScene.updateFrame([context](Scene::Entity::SkeletonPtr skeleton) {
                if (!skeleton->mBoneMatrices.mBuffer) {
                    skeleton->mBoneMatrices = context->allocateBuffer<Matrix4[]>(skeleton->data()->mBones.size());
                }
                return context->mapBuffer(skeleton->mBoneMatrices); });
        });

        co_return {};
    }

}
}