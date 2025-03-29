#include "../scenerendererlib.h"

#include "pointshadowrenderpass.h"

#include "Madgine/scene/scenemanager.h"

#include "Madgine/scene/entity/components/mesh.h"
#include "Madgine/scene/entity/components/pointlight.h"
#include "Madgine/scene/entity/components/skeleton.h"
#include "Madgine/scene/entity/components/transform.h"
#include "Madgine/scene/entity/entity.h"

#include "Madgine/render/rendercontext.h"
#include "Madgine/render/rendertarget.h"

#include "scenemainwindowcomponent.h"

#include "Meta/math/transformation.h"

#include "Meta/math/frustum.h"

//#include "Madgine/render/rendercontext.h"

#include "Madgine/render/shadinglanguage/sl_support_begin.h"
#include "shaders/pointshadow.sl"
#include "Madgine/render/shadinglanguage/sl_support_end.h"


namespace Engine {
namespace Render {

    PointShadowRenderPass::PointShadowRenderPass(size_t index, SceneMainWindowComponent &scene, int priority)
        : mData(scene, nullptr)
        , mIndex(index)
        , mPriority(priority)
    {
    }

    void PointShadowRenderPass::setup(RenderTarget *target)
    {
        mPipeline.create({ .vs = "pointshadow", .ps = "pointshadow", .bufferSizes = { sizeof(PointShadowPerApplication), 0, 0 } });

        addDependency(&mData);
    }

    void PointShadowRenderPass::shutdown(RenderTarget *target)
    {
        removeDependency(&mData);

        mPipeline.reset();
    }

    void PointShadowRenderPass::render(Render::RenderTarget *target, size_t iteration)
    {
        if (!mPipeline.available())
            return;

        Scene::Entity::EntityComponentList<Scene::Entity::PointLight> &lights = mData.mScene.scene()->entityComponentList<Scene::Entity::PointLight>();
        if (mIndex >= lights.size())
            return;

        //TODO Culling

        Scene::Entity::Entity *entity = lights[mIndex].entity();
        Scene::Entity::Transform *transform = entity->getComponent<Scene::Entity::Transform>();
        if (!transform)
            return;

        {
            auto perApplication = mPipeline->mapParameters<PointShadowPerApplication>(0);

            Frustum f {
                Vector3::ZERO,
                Quaternion {},
                0.01f, 0.01f,
                0.01f, 100.0f,
                false
            };
            perApplication->p = ProjectionMatrix(f);
        }

        for (std::pair<const GPUMeshData *const, std::vector<ShadowSceneRenderData::ObjectData>> &instance : mData.mInstances) {
            const GPUMeshData *meshData = instance.first;

            std::vector<PointShadowInstanceData> instanceData;

            
            static constexpr Matrix4 rotationMatrices[] = {
                { 0, 0, -1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1 },
                { 0, 0, 1, 0, 0, 1, 0, 0, -1, 0, 0, 0, 0, 0, 0, 1 },
                { 1, 0, 0, 0, 0, 0, -1, 0, 0, 1, 0, 0, 0, 0, 0, 1 },
                { 1, 0, 0, 0, 0, 0, 1, 0, 0, -1, 0, 0, 0, 0, 0, 1 },
                { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 },
                { -1, 0, 0, 0, 0, 1, 0, 0, 0, 0, -1, 0, 0, 0, 0, 1 }
            };

            Matrix4 v = rotationMatrices[iteration] * TranslationMatrix(-transform->mPosition);
            
            {
                auto instanceData = mPipeline->mapTempBuffer<PointShadowInstanceData[]>(1, instance.second.size());

                std::ranges::transform(instance.second, instanceData.mData, [&](const ShadowSceneRenderData::ObjectData &o) {
                    Matrix4 mv = v * o.mTransform;
                    return PointShadowInstanceData {
                        mv.Transpose(),
                        o.mBones
                    };
                });
            }

            mPipeline->bindMesh(target, meshData);
            mPipeline->renderInstanced(target, instance.second.size());
        }
    }

    int PointShadowRenderPass::priority() const
    {
        return mPriority;
    }

    size_t PointShadowRenderPass::iterations() const
    {
        return 6;
    }

    size_t PointShadowRenderPass::targetSubresourceIndex(size_t iteration) const
    {
        return iteration;
    }

    size_t PointShadowRenderPass::targetCount(size_t) const
    {
        return 0;
    }

    std::string_view PointShadowRenderPass::name() const
    {
        return "PointShadow";
    }

}
}
