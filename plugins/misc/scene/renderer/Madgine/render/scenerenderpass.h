#pragma once

#include "Madgine/render/renderpass.h"

#include "Madgine/pipelineloader/pipelineloader.h"

#include "pointshadowrenderpass.h"
#include "shadowrenderpass.h"

#include "litscenerenderdata.h"

namespace Engine {
namespace Render {

    struct MADGINE_SCENE_RENDERER_EXPORT SceneRenderPass : RenderPass {
        SceneRenderPass(SceneMainWindowComponent &scene, Camera *camera, int priority);
        SceneRenderPass(SceneRenderPass &&);
        ~SceneRenderPass();

        virtual void setup(RenderTarget *target) override;
        virtual void shutdown(RenderTarget *target) override;
        virtual void render(Render::RenderTarget *target, size_t iteration) override;

        virtual int priority() const override;
        virtual size_t targetCount(size_t) const override;

        virtual std::string_view name() const override;

        virtual void debugCameras(Closure<void(const Camera &, std::string_view)> handler) const override;

        float mAmbientFactor = 1.0f;
        float mDiffuseFactor = 1.0f;
        float mSpecularFactor = 1.0f;

        float mLightConstantFactor = 1.0f;
        float mLightLinearFactor = 4.5f;
        float mLightSquaredFactor = 75.0f;

    private:
        PipelineLoader::Instance mPipeline;

        LitSceneRenderData mData;

        ShadowRenderPass mShadowPass;

        std::unique_ptr<Render::RenderTarget> mShadowMap;

        UniqueResourceBlock mShadowResourceBlock;

        int mPriority;
    };

}
}