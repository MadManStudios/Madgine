#pragma once

#include "Madgine/render/renderpass.h"

#include "Madgine/pipelineloader/pipelineloader.h"

#include "Meta/math/frustum.h"

#include "shadowscenerenderdata.h"

namespace Engine {
namespace Render {

    struct MADGINE_SCENE_RENDERER_EXPORT ShadowRenderPass : RenderPass {
        ShadowRenderPass(SceneMainWindowComponent &scene, Render::Camera *camera, int priority);

        virtual void setup(RenderTarget *target) override;
        virtual void shutdown(RenderTarget *target) override;
        virtual void render(RenderTarget *target, size_t iteration) override;        

        virtual int priority() const override;
        virtual size_t targetCount(size_t) const override;

        virtual std::string_view name() const override;

        Matrix4 projectionMatrix() const;
        Matrix4 viewMatrix() const;
        Matrix4 viewProjectionMatrix() const;

        void updateFrustum(float aspectRatio);

        virtual void debugFrustums(Closure<void(const Frustum &, std::string_view)> handler) const override;

    private:
        PipelineLoader::Instance mPipeline;

        ShadowSceneRenderData mData;

        int mPriority;

        Frustum mLightFrustum;
    };

}
}