#include "../scenerendererlib.h"

#include "scenerenderpass.h"

#include "Meta/math/transformation.h"

#include "Madgine/render/camera.h"
#include "Madgine/render/rendercontext.h"
#include "Madgine/render/rendertarget.h"
#include "Madgine/render/shadercache.h"
#include "Madgine/scene/entity/components/material.h"
#include "Madgine/scene/entity/components/mesh.h"
#include "Madgine/scene/entity/components/pointlight.h"
#include "Madgine/scene/entity/components/skeleton.h"
#include "Madgine/scene/entity/components/transform.h"
#include "Madgine/scene/entity/entity.h"
#include "Madgine/scene/scenemanager.h"

#include "Meta/keyvalue/metatable_impl.h"

#include "light_hlsl.h"
#include "scene_hlsl.h"
#include "scenemainwindowcomponent.h"

METATABLE_BEGIN(Engine::Render::SceneRenderPass)
    MEMBER(mLightConstantFactor)
    MEMBER(mLightLinearFactor)
    MEMBER(mLightSquaredFactor)
METATABLE_END(Engine::Render::SceneRenderPass)

CACHED_SHADER(HLSL::scene | HLSL::lighting)

namespace Engine {
namespace Render {

    SceneRenderPass::SceneRenderPass(Scene::SceneManager &scene, SceneRenderData &renderData, PointShadowRenderData &pointShadowRenderData, Camera &camera, int priority)
        : mData(scene, renderData, camera)
        , mPriority(priority)
        , mShadowPass(scene, renderData, camera, 50)
        , mPointShadowRenderData(pointShadowRenderData)
    {
    }

    SceneRenderPass::~SceneRenderPass() = default;

    void SceneRenderPass::setup(RenderTarget *target)
    {
        mShadowMap = target->context()->createRenderTexture({ 2048, 2048 }, { .mName = "ShadowMap", .mType = TextureType_2DMultiSample, .mCreateDepthBufferView = true, .mSamples = 4, .mTextureCount = 0 });

        mShadowMap->addRenderPass(&mShadowPass);

        setupImpl(target, HLSL::scene_VS, HLSL::scene | HLSL::lighting, { sizeof(HLSL::ScenePerApplication), sizeof(HLSL::LightPerFrame), sizeof(HLSL::ScenePerObject) });

        addDependency(&mData);
        addDependency(mShadowMap.get());
        addDependency(&mPointShadowRenderData);

        std::vector<std::variant<ConstTexturePtr, GPUPtr<void>, GPUPtr<Void[]>>> textures;
        std::ranges::move(mPointShadowRenderData.depthTextures(), std::back_inserter(textures));        
        textures.insert(textures.begin(), mShadowMap->depthTexture());
        mShadowResourceBlock = target->context()->createResourceBlock(std::move(textures));

    }

    void SceneRenderPass::shutdown(RenderTarget *target)
    {
        removeDependency(&mData);
        removeDependency(mShadowMap.get());
        removeDependency(&mPointShadowRenderData);

        RenderPass::shutdown(target);

        mShadowMap.reset();

        target->context()->destroyResourceBlock(mShadowResourceBlock);
    }

    void SceneRenderPass::render(Render::RenderTarget *target, size_t iteration)
    {
        if (!mPipeline.available())
            return;

        Vector2i size = target->size();

        float aspectRatio = float(size.x) / size.y;

        {
            auto perApplication = mPipeline->mapParameters<HLSL::ScenePerApplication>(0);

            perApplication->p = target->getClipSpaceMatrix() * mData.mCamera.getProjectionMatrix(aspectRatio);
        }

        Matrix4 v = mData.mCamera.getViewMatrix();

        {
            auto perFrame = mPipeline->mapParameters<HLSL::LightPerFrame>(1);

            perFrame->light.caster.reprojectionMatrix = mShadowPass.viewProjectionMatrix() * v.Inverse();

            perFrame->light.caster.shadowSamples = 4;

            perFrame->light.light.color = Vector3 { mData.mScene.mAmbientLightColor };
            perFrame->light.light.dir = (v * Vector4 { mData.mScene.mAmbientLightDirection, 0.0f }).xyz();
            perFrame->light.light.orthographic = mData.mScene.mAmbientLightOrthographic;

            Scene::Entity::EntityComponentList<Scene::Entity::PointLight> &lights = mData.mScene.entityComponentList<Scene::Entity::PointLight>();
            perFrame->pointLightCount = lights.size();
            if (perFrame->pointLightCount > 2) {
                LOG_WARNING("Too many point lights in scene!");
                perFrame->pointLightCount = 2;
            }

            size_t i = 0;
            for (Scene::Entity::PointLight &light : lights | std::ranges::views::take(2)) {
                Scene::Entity::Transform *t = light.entity().getComponent<Scene::Entity::Transform>();
                if (t) {
                    float range = light.mRange;
                    perFrame->pointLights[i].light.position = (v * Vector4 { t->mPosition, 1.0f }).xyz();
                    perFrame->pointLights[i].light.color = Vector3 { light.mColor };
                    perFrame->pointLights[i].light.constantFactor = mLightConstantFactor;
                    perFrame->pointLights[i].light.linearFactor = mLightLinearFactor / range;
                    perFrame->pointLights[i].light.squaredFactor = mLightSquaredFactor / (range * range);
                    perFrame->pointLights[i].caster.reprojectionMatrix = v.Inverse();
                }
                ++i;
            }
        }

        mPipeline->bindResources(target, 3, mShadowResourceBlock);

        for (const std::pair<const LitSceneRenderData::NonInstancedData, std::vector<LitSceneRenderData::ObjectData>> &instance : mData.mInstances) {
            const GPUMeshData *meshData = instance.first.mMesh;
            ResourceBlock material = instance.first.mMaterial;

            {
                auto perObject = mPipeline->mapParameters<HLSL::ScenePerObject>(2);

                perObject->hasDistanceField = false;

                perObject->hasTexture = material;
            }

            if (material)
                mPipeline->bindResources(target, 2, material);
            else
                mPipeline->bindResources(target, 2, mData.mRenderData.defaultMaterial());

            {
                auto instanceData = mPipeline->mapTempBuffer<HLSL::SceneInstanceData[]>(1, instance.second.size());

                std::ranges::transform(instance.second, instanceData.mData, [&](const LitSceneRenderData::ObjectData &o) {
                    Matrix4 mv = v * o.mTransform;
                    return HLSL::SceneInstanceData {
                        mv.Transpose(),
                        mv.Inverse() /*.Transpose().Transpose()*/,
                        o.mDiffuseColor,
                        Vector4 { 1.0f, 1.0f, 1.0f, 1.0f },
                        // o.mBones
                    };
                });
            }

            mPipeline->bindMesh(target, *meshData);
            mPipeline->renderInstanced(target, instance.second.size());
            // mPipeline->renderMeshInstanced(target, std::move(instanceData), meshData);
        }
    }

    int SceneRenderPass::priority() const
    {
        return mPriority;
    }

    size_t SceneRenderPass::targetCount(size_t) const
    {
        return 2;
    }

    std::string_view SceneRenderPass::name() const
    {
        return "Scene";
    }

    void SceneRenderPass::debugCameras(CallableView<void(const Camera &, std::string_view)> handler) const
    {
        handler(mData.mCamera, "SceneRenderPass");
    }

}
}
