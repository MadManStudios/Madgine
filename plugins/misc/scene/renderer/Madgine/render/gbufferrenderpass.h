#pragma once

#include "Madgine/render/renderpass.h"

#include "litscenerenderdata.h"

namespace Engine {
namespace Render {

    // Renders the surface attributes of all meshes of a scene into a G-buffer, as input for a custom lighting pass. The
    // target it is added to needs three colour textures and a depth buffer, optionally multisampled, e.g.
    //   context->createRenderTexture(size, { .mType = TextureType_2DMultiSample, .mFormat = FORMAT_RGBA16F,
    //       .mCreateDepthBufferView = true, .mSamples = 4, .mTextureCount = 3 });
    // Layout (all positions and directions in the view space of the camera):
    //   texture 0: albedo.rgb, 1
    //   texture 1: normal (octahedral encoded, see shadinglanguage/normalencoding.hlsl).xy, shininess, 1
    //   texture 2: emissive.rgb, 1
    //   depth:     depth of the camera projection
    // Surfaces are not blended: (mostly) transparent pixels are cut out.
    struct MADGINE_SCENE_RENDERER_EXPORT GBufferRenderPass : RenderPass {
        GBufferRenderPass(Scene::SceneManager &scene, SceneRenderData &renderData, Camera &camera, int priority);

        void setup(RenderTarget *target) override;
        void shutdown(RenderTarget *target) override;
        void render(RenderTarget *target, size_t iteration) override;

        int priority() const override;
        size_t targetCount(size_t iteration) const override;
        std::string_view name() const override;

        void debugCameras(CallableView<void(const Camera &, std::string_view)> handler) const override;

    private:
        LitSceneRenderData mData;

        int mPriority;

        std::vector<const LitSceneRenderData::ObjectData *> mVisible;
    };

}
}
