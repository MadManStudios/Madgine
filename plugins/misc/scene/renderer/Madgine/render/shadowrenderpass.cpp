#include "../scenerendererlib.h"

#include "shadowrenderpass.h"

#include "Madgine/render/camera.h"
#include "Madgine/render/rendercontext.h"
#include "Madgine/render/rendertarget.h"
#include "Madgine/scene/entity/components/mesh.h"
#include "Madgine/scene/entity/components/skeleton.h"
#include "Madgine/scene/entity/components/transform.h"
#include "Madgine/scene/entity/entity.h"
#include "Madgine/scene/scenemanager.h"

#include "frustumculling.h"
#include "scenemainwindowcomponent.h"

// #include "Madgine/render/rendercontext.h"

#include "scene_hlsl.h"
#include "light_hlsl.h"

namespace Engine {
namespace Render {

    ShadowRenderPass::ShadowRenderPass(Scene::SceneManager &scene, SceneRenderData &renderData, Render::Camera &camera, int priority)
        : mData(scene, renderData)
        , mCamera(camera)
        , mPriority(priority)
    {
    }

    void ShadowRenderPass::setup(RenderTarget *target)
    {
        setupImpl(target, HLSL::scene_VS, {}, { sizeof(HLSL::ScenePerApplication), sizeof(HLSL::LightPerFrame), sizeof(HLSL::ScenePerObject) });

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

        updateFrustum(mAspectRatio);

        {
            auto perApplication = mPipeline->mapParameters<HLSL::ScenePerApplication>(0);

            perApplication->p = target->getClipSpaceMatrix() * projectionMatrix();
        }

        Math::Matrix4 v = viewMatrix();
        // The light frustum already reaches towards the sun as far as casters can throw shadows into the view
        FrustumCuller culler { mLightFrustum };

        {
            auto perFrame = mPipeline->mapParameters<HLSL::LightPerFrame>(1);

            perFrame->light.light.color = Math::Vector3 { mData.mScene.mSunLightColor };
            perFrame->light.light.dir = (v * Math::Vector4 { mData.mScene.mSunLightDirection, 0.0f }).xyz();
        }

        for (const std::pair<GPUMeshLoader::Handle const, std::vector<ShadowSceneRenderData::ObjectData>> &instance : mData.mInstances) {
            GPUMeshLoader::Handle meshData = instance.first;

            // Only the instances whose bounding box reaches into the light frustum (skinned meshes always)
            mVisible.clear();
            for (const ShadowSceneRenderData::ObjectData &o : instance.second)
                if (o.mBones || culler.visible(meshData->mAABB, o.mTransform))
                    mVisible.push_back(&o);
            if (mVisible.empty())
                continue;

            {
                auto perObject = mPipeline->mapParameters<HLSL::ScenePerObject>(2);

                perObject->hasDistanceField = false;

                perObject->hasTexture = false;
            }

            {
                auto instanceData = mPipeline->mapTempBuffer<HLSL::SceneInstanceData[]>(1, mVisible.size());

                std::ranges::transform(mVisible, instanceData.mData, [&](const ShadowSceneRenderData::ObjectData *o) {
                    Math::Matrix4 mv = v * o->mTransform;
                    return HLSL::SceneInstanceData {
                        mv.Transpose(),
                        mv.Inverse().Transpose().Transpose(),
                        Math::Vector4 { 0.0f, 0.0f, 0.0f, 0.0f },
                        Math::Vector4 { 0.0f, 0.0f, 0.0f, 0.0f },
                        //o.mBones
                    };
                });
            }

            mPipeline->bindMesh(target, *meshData);

            mPipeline->renderInstanced(target, mVisible.size());
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

    Math::Matrix4 ShadowRenderPass::projectionMatrix() const
    {
        return mLightFrustum.getProjectionMatrix();
    }

    Math::Matrix4 ShadowRenderPass::viewMatrix() const
    {
        return mLightFrustum.getViewMatrix();
    }

    Math::Matrix4 ShadowRenderPass::viewProjectionMatrix() const
    {
        return mLightFrustum.getViewProjectionMatrix();
    }

    // Points spanning the part of the frustum inside the bounds: the frustum faces clipped against the box, plus the box
    // corners inside the frustum. Every vertex of the intersection is among them.
    static std::vector<Math::Vector3> clipFrustum(const std::array<Math::Vector3, 8> &corners, const Math::AABB &bounds)
    {
        static constexpr int faces[6][4] = { { 0, 1, 2, 3 }, { 4, 5, 6, 7 }, { 0, 1, 5, 4 }, { 1, 2, 6, 5 }, { 2, 3, 7, 6 }, { 3, 0, 4, 7 } };

        std::vector<Math::Vector3> points;
        std::vector<Math::Vector3> poly, clipped;
        for (const auto &face : faces) {
            poly.assign({ corners[face[0]], corners[face[1]], corners[face[2]], corners[face[3]] });
            for (size_t axis = 0; axis < 3 && !poly.empty(); ++axis) {
                for (int side = 0; side < 2 && !poly.empty(); ++side) {
                    float limit = side == 0 ? bounds.mMin[axis] : bounds.mMax[axis];
                    float sign = side == 0 ? 1.0f : -1.0f;
                    clipped.clear();
                    for (size_t i = 0; i < poly.size(); ++i) {
                        const Math::Vector3 &p0 = poly[i];
                        const Math::Vector3 &p1 = poly[(i + 1) % poly.size()];
                        float d0 = sign * (p0[axis] - limit);
                        float d1 = sign * (p1[axis] - limit);
                        if (d0 >= 0.0f)
                            clipped.push_back(p0);
                        if ((d0 >= 0.0f) != (d1 >= 0.0f))
                            clipped.push_back(p0 + (p1 - p0) * (d0 / (d0 - d1)));
                    }
                    std::swap(poly, clipped);
                }
            }
            points.insert(points.end(), poly.begin(), poly.end());
        }

        Math::Vector3 centre = std::accumulate(corners.begin(), corners.end(), Math::Vector3 { Math::Vector3::ZERO }) / 8.0f;
        for (const Math::Vector3 &p : bounds.corners()) {
            bool inside = true;
            for (const auto &face : faces) {
                Math::Vector3 o = corners[face[0]];
                Math::Vector3 n = (corners[face[1]] - o).crossProduct(corners[face[3]] - o);
                if (n.dotProduct(centre - o) * n.dotProduct(p - o) < 0.0f) {
                    inside = false;
                    break;
                }
            }
            if (inside)
                points.push_back(p);
        }

        return points;
    }

    void ShadowRenderPass::updateFrustum(float aspectRatio)
    {
        Math::Vector3 minBounds = std::numeric_limits<float>::max() * Math::Vector3 { Math::Vector3::UNIT_SCALE };
        Math::Vector3 maxBounds = std::numeric_limits<float>::lowest() * Math::Vector3 { Math::Vector3::UNIT_SCALE };

        // The light space turns with the camera for the best fit of the view in the shadow map
        const Math::NormalizedVector3 &lightDir = mData.mScene.mSunLightDirection;
        Math::Quaternion q = Math::Quaternion::FromDirection(lightDir, (mCamera.mOrientation * Math::Vector3 { Math::Vector3::UNIT_Z }).crossProduct(lightDir));
        Math::Quaternion qInv = q.inverse();

        std::array<Math::Vector3, 8> corners = mCamera.getFrustum(aspectRatio).getCorners();
        std::vector<Math::Vector3> points;
        float margin = 1.0f;
        if (mBounds) {
            points = clipFrustum(corners, *mBounds);
            // Casters outside the view still throw shadows into it: extend the light frustum towards the light until it
            // leaves the bounds
            float height = mBounds->mMax.y - mBounds->mMin.y;
            margin = std::min(height / std::max(std::abs(lightDir.y), 0.1f), 2.0f * mBounds->diameter()) + 1.0f;
        }
        if (points.empty())
            points.assign(corners.begin(), corners.end());

        for (const Math::Vector3 &p : points) {
            Math::Vector3 local = qInv * p;
            minBounds = min(minBounds, local);
            maxBounds = max(maxBounds, local);
        }

        // Quantize the extent (steps of 1/8 of a power of two) and snap the origin to whole texels, so that moving the
        // camera without turning it only shifts the shadow map by whole texels and the shadows stay still
        auto quantize = [](float size) {
            float step = std::exp2(std::floor(std::log2(std::max(size, 1.0f)))) / 8.0f;
            return std::ceil(size / step) * step;
        };
        Math::Vector2 size;
        Math::Vector2 origin;
        for (size_t axis = 0; axis < 2; ++axis) {
            float extent = maxBounds[axis] - minBounds[axis];
            float texel = quantize(extent * (1.0f + 2.0f / sShadowMapSize)) / (sShadowMapSize - 2.0f);
            size[axis] = texel * sShadowMapSize;
            origin[axis] = (std::floor(minBounds[axis] / texel) - 1.0f) * texel;
        }

        Math::Vector3 relPos { origin.x + size.x / 2.0f, origin.y + size.y / 2.0f, minBounds.z - margin };

        mLightFrustum = {
            q * relPos,
            q,
            size.y / 2.0f, size.x / 2.0f,
            0.0f, maxBounds.z - minBounds.z + margin,
            true
        };
    }

    void ShadowRenderPass::debugFrustums(CallableView<void(const Math::Frustum &, std::string_view)> handler) const
    {
        handler(mLightFrustum, "ShadowRenderPass");
    }

}
}
