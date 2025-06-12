#include "../scenerendererlib.h"

#include "shadowrenderpass.h"

#include "Madgine/scene/scenemanager.h"

#include "Madgine/scene/entity/components/mesh.h"
#include "Madgine/scene/entity/components/skeleton.h"
#include "Madgine/scene/entity/components/transform.h"
#include "Madgine/scene/entity/entity.h"

#include "Madgine/render/rendercontext.h"
#include "Madgine/render/rendertarget.h"

#include "Madgine/render/camera.h"

#include "scenemainwindowcomponent.h"

// #include "Madgine/render/rendercontext.h"

#include "Madgine/render/shadinglanguage/sl_support_begin.h"
#include "shaders/scene.sl"
#include "Madgine/render/shadinglanguage/sl_support_end.h"

#include "scene_hlsl.h"

namespace Engine {
namespace Render {

    ShadowRenderPass::ShadowRenderPass(SceneMainWindowComponent &scene, Render::Camera *camera, int priority)
        : mData(scene, camera)
        , mPriority(priority)
    {
    }

    void ShadowRenderPass::setup(RenderTarget *target)
    {
        setupImpl(target, HLSL::scene_VS, {}, { sizeof(ScenePerApplication), sizeof(ScenePerFrame), sizeof(ScenePerObject) });

        addDependency(&mData);
    }

    void ShadowRenderPass::shutdown(RenderTarget *target)
    {
        removeDependency(&mData);

        RenderPass::shutdown(target);
    }

    void ShadowRenderPass::render(Render::RenderTarget *target, size_t iteration)
    {

        if (!mPipeline.available())
            return;

        Vector2i size = target->size();
        updateFrustum(1.0f);

        {
            auto perApplication = mPipeline->mapParameters<ScenePerApplication>(0);

            perApplication->p = target->getClipSpaceMatrix() * projectionMatrix();
        }

        Matrix4 v = viewMatrix();

        {
            auto perFrame = mPipeline->mapParameters<ScenePerFrame>(1);

            perFrame->light.light.color = mData.mScene.mAmbientLightColor;
            perFrame->light.light.dir = (v * Vector4 { mData.mScene.mAmbientLightDirection, 0.0f }).xyz();
        }

        for (const std::pair<const GPUMeshData *, std::vector<ShadowSceneRenderData::ObjectData>> &instance : mData.mInstances) {
            const GPUMeshData *meshData = instance.first;

            {
                auto perObject = mPipeline->mapParameters<ScenePerObject>(2);

                perObject->hasLight = false;

                perObject->hasDistanceField = false;

                perObject->hasTexture = false;
            }

            std::vector<SceneInstanceData> instanceData;

            {
                auto instanceData = mPipeline->mapTempBuffer<SceneInstanceData[]>(1, instance.second.size());

                std::ranges::transform(instance.second, instanceData.mData, [&](const ShadowSceneRenderData::ObjectData &o) {
                    Matrix4 mv = v * o.mTransform;
                    return SceneInstanceData {
                        mv.Transpose(),
                        mv.Inverse().Transpose().Transpose(),
                        Vector4 { 0.0f, 0.0f, 0.0f, 0.0f },
                        Vector4 { 0.0f, 0.0f, 0.0f, 0.0f },
                        o.mBones
                    };
                });
            }

            mPipeline->bindMesh(target, meshData);

            mPipeline->renderInstanced(target, instance.second.size());
            // mPipeline->renderMeshInstanced(target, std::move(instanceData), meshData);
        }
    }

    int ShadowRenderPass::priority() const
    {
        return mPriority;
    }

    size_t ShadowRenderPass::targetCount(size_t) const
    {
        return 0;
    }

    std::string_view ShadowRenderPass::name() const
    {
        return "Shadow";
    }

    Matrix4 ShadowRenderPass::projectionMatrix() const
    {
        return mLightFrustum.getProjectionMatrix();
    }

    Matrix4 ShadowRenderPass::viewMatrix() const
    {
        return mLightFrustum.getViewMatrix();
    }

    Matrix4 ShadowRenderPass::viewProjectionMatrix() const
    {
        return mLightFrustum.getViewProjectionMatrix();
    }

    void ShadowRenderPass::updateFrustum(float aspectRatio)
    {
        Vector3 minBounds = std::numeric_limits<float>::max() * Vector3 { Vector3::UNIT_SCALE };
        Vector3 maxBounds = std::numeric_limits<float>::lowest() * Vector3 { Vector3::UNIT_SCALE };

        Quaternion q = Quaternion::FromDirection(mData.mScene.mAmbientLightDirection);
        Quaternion qInv = q.inverse();

        Frustum cameraFrustum = mData.mCamera->getFrustum(aspectRatio);
        Frustum localFrustum = qInv * cameraFrustum;
        auto corners = localFrustum.getCorners();

        minBounds = std::accumulate(corners.begin(), corners.end(), minBounds, [](const Vector3 &v1, const Vector3 &v2) { return min(v1, v2); });
        maxBounds = std::accumulate(corners.begin(), corners.end(), maxBounds, [](const Vector3 &v1, const Vector3 &v2) { return max(v1, v2); });

        Vector3 relPos = (maxBounds + minBounds) / 2.0f;
        relPos.z = minBounds.z - 1.0f;

        Vector2 size = maxBounds.xy() - minBounds.xy();

        mLightFrustum = {
            q * relPos,
            q,
            size.y / 2.0f, size.x / 2.0f,
            1.0f, maxBounds.z - minBounds.z + 1.0f,
            true
        };
    }

    void ShadowRenderPass::debugFrustums(Closure<void(const Frustum &, std::string_view)> handler) const
    {
        handler(mLightFrustum, "ShadowRenderPass");
    }

}
}
