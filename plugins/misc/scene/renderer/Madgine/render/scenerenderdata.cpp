#include "../scenerendererlib.h"

#include "scenerenderdata.h"

#include "Generic/containers/safeiterator.h"

#include "Madgine/render/rendercontext.h"
#include "Madgine/scene/behavior/animation.h"
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
    }

    Threading::ImmediateTask<RenderFuture> SceneRenderData::render(RenderContext *context)
    {
        co_await mScene.updateFrame([context](Scene::Entity::Skeleton *skeleton) -> Memory::TypedByteBuffer<Math::Matrix4[]> {
            throw "TODO";
                /*if (!skeleton->mBoneMatrices) {
                    skeleton->mBoneMatrices = context->allocateBuffer<Math::Matrix4[]>(skeleton->data()->mBones.size());
                }
                return context->mapBuffer(skeleton->mBoneMatrices);*/ });

        co_return {};
    }

    void SceneRenderData::setup(RenderContext *context)
    {
        HLSL::sceneResourceBlock0 block;
        block.material./* emplace_back().*/shininess = 128.0f;
        mDefaultMaterial = block.toResourceBlock(context).first;
    }

    void SceneRenderData::shutdown(RenderContext *context)
    {
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

}
}