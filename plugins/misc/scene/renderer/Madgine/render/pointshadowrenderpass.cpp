#include "../scenerendererlib.h"

#include "pointshadowrenderpass.h"

#include "Meta/math/frustum.h"
#include "Meta/math/transformation.h"

#include "Madgine/render/rendercontext.h"
#include "Madgine/render/rendertarget.h"
#include "Madgine/scene/entity/components/mesh.h"
#include "Madgine/scene/entity/components/pointlight.h"
#include "Madgine/scene/entity/components/skeleton.h"
#include "Madgine/scene/entity/components/transform.h"
#include "Madgine/scene/entity/entity.h"
#include "Madgine/scene/scenemanager.h"

#include "frustumculling.h"
#include "scenemainwindowcomponent.h"

// #include "Madgine/render/rendercontext.h"

#include "pointshadow_hlsl.h"

namespace Engine {
namespace Render {

    PointShadowRenderPass::PointShadowRenderPass(size_t index, Scene::SceneManager &scene, SceneRenderData &renderData, int priority)
        : mData(scene, renderData)
        , mIndex(index)
        , mPriority(priority)
    {
    }

    void PointShadowRenderPass::setup(RenderTarget *target)
    {
        setupImpl(target, HLSL::pointshadow_VS, HLSL::pointshadow_PS, { sizeof(HLSL::PointShadowPerApplication), 0, 0 });

        addDependency(&mData);
    }

    void PointShadowRenderPass::shutdown(RenderTarget *target)
    {
        removeDependency(&mData);

        RenderPass::shutdown(target);
    }

    void PointShadowRenderPass::render(Render::RenderTarget *target, size_t iteration)
    {
        if (!mPipeline.available())
            return;

        // The mIndex-th shadow casting light that is switched on (same selection as SceneRenderPass)
        Scene::Entity::Entity *lightEntity = nullptr;
        size_t index = 0;
        for (auto &[light, entity] : mData.mScene.entityComponentList<Scene::Entity::PointLight>()) {
            if (light.castsShadow() && index++ == mIndex) {
                lightEntity = &entity;
                break;
            }
        }
        if (!lightEntity)
            return;

        Scene::Entity::Entity &entity = *lightEntity;
        Scene::Entity::Transform *transform = entity.getComponent<Scene::Entity::Transform>();
        if (!transform)
            return;

        Math::Frustum f {
            Math::Vector3::ZERO,
            Math::Quaternion {},
            0.01f, 0.01f,
            0.01f, 100.0f,
            false
        };

        {
            auto perApplication = mPipeline->mapParameters<HLSL::PointShadowPerApplication>(0);

            perApplication->p = ProjectionMatrix(f);
        }

        static constexpr Math::Quaternion rotationQuaternions[] = {
            { 0, 0.70710678f, 0, 0.70710678f }, // +90° around Y
            { 0, -0.70710678f, 0, 0.70710678f }, // -90° around Y
            { -0.70710678f, 0, 0, 0.70710678f }, // -90° around X
            { 0.70710678f, 0, 0, 0.70710678f }, // +90° around X            
            { 0, 0, 0, 1 }, // Identity
            { 0, 1, 0, 0 } // 180° around Y
        };

        Math::Vector3 worldPos = transform->worldPosition(entity);

        Math::Matrix4 v = Math::Matrix4 { (-rotationQuaternions[iteration]).toMatrix() } * TranslationMatrix(-worldPos);

        Math::Frustum rotatedFrustum = rotationQuaternions[iteration] * f;
        rotatedFrustum.mPosition = worldPos;
        FrustumCuller culler { rotatedFrustum };

        for (const auto &[meshData, objects] : mData.mInstances) {

            // Only the instances whose bounding box reaches into the view (skinned meshes always)
            mVisible.clear();
            const Math::AABB &aabb = meshData->mAABB;
            for (const ShadowSceneRenderData::ObjectData &o : objects)
                if (o.mBones || culler.visible(aabb, o.mTransform))
                    mVisible.push_back(&o);
            if (mVisible.empty())
                continue;

            {
                auto instanceData = mPipeline->mapTempBuffer<HLSL::PointShadowInstanceData[]>(1, mVisible.size());

                std::ranges::transform(mVisible, instanceData.mData, [&](const ShadowSceneRenderData::ObjectData *o) {
                    Math::Matrix4 mv = v * o->mTransform;
                    return HLSL::PointShadowInstanceData {
                        mv.Transpose(),
                        // o.mBones
                    };
                });
            }

            mPipeline->bindMesh(target, *meshData);
            mPipeline->renderInstanced(target, mVisible.size());
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
