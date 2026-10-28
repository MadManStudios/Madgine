#pragma once

#include "Meta/math/boundingbox.h"
#include "Meta/math/frustum.h"

#include "Madgine/render/pipelineloader.h"
#include "Madgine/render/renderpass.h"

#include "shadowscenerenderdata.h"

namespace Engine {
namespace Render {

    struct MADGINE_SCENE_RENDERER_EXPORT ShadowRenderPass : RenderPass {
        ShadowRenderPass(Scene::SceneManager &scene, SceneRenderData &renderData, Render::Camera &camera, int priority);

        virtual void setup(RenderTarget *target) override;
        virtual void shutdown(RenderTarget *target) override;
        virtual void render(RenderTarget *target, size_t iteration) override;

        virtual int priority() const override;
        virtual size_t targetCount(size_t) const override;

        virtual std::string_view name() const override;

        Math::Matrix4 projectionMatrix() const;
        Math::Matrix4 viewMatrix() const;
        Math::Matrix4 viewProjectionMatrix() const;

        void updateFrustum(float aspectRatio);

        // Aspect ratio of the view the shadows are rendered for (set by the scene pass)
        float mAspectRatio = 1.0f;

        // Optional world-space bounds of all shadow receivers and casters. When set, the shadow map only covers the part
        // of the camera frustum inside these bounds (e.g. the ground layer of a city instead of the whole far plane).
        std::optional<Math::AABB> mBounds;

        // Resolution of the shadow map, used to snap the light frustum to whole texels
        static constexpr float sShadowMapSize = 2048.0f;

        virtual void debugFrustums(CallableView<void(const Math::Frustum &, std::string_view)> handler) const override;

    private:
        ShadowSceneRenderData mData;

        Render::Camera &mCamera;

        int mPriority;

        Math::Frustum mLightFrustum;

        std::vector<const ShadowSceneRenderData::ObjectData *> mVisible;
    };

}
}