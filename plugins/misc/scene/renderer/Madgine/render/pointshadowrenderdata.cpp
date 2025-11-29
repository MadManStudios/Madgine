#include "../scenerendererlib.h"

#include "pointshadowrenderdata.h"

#include "Madgine/render/rendercontext.h"

#include "Madgine/render/rendertarget.h"

namespace Engine {
namespace Render {

    PointShadowRenderData::PointShadowRenderData(Scene::SceneManager &scene, SceneRenderData &sceneData)
        : mPointShadowPasses { { 0, scene, sceneData, 50 }, { 1, scene, sceneData, 50 } }
    {
    }

    PointShadowRenderData::~PointShadowRenderData() = default;

    void PointShadowRenderData::setup(RenderContext *context)
    {
        mPointShadowMaps[0] = context->createRenderTexture({ 2048, 2048 }, { .mName = "PointShadowMap0", .mType = TextureType_Cube, .mCreateDepthBufferView = true, .mTextureCount = 0 });
        mPointShadowMaps[1] = context->createRenderTexture({ 2048, 2048 }, { .mName = "PointShadowMap1", .mType = TextureType_Cube, .mCreateDepthBufferView = true, .mTextureCount = 0 });

        mPointShadowMaps[0]->addRenderPass(&mPointShadowPasses[0]);
        mPointShadowMaps[1]->addRenderPass(&mPointShadowPasses[1]);
    }

    void PointShadowRenderData::shutdown(RenderContext *context)
    {
        mPointShadowMaps[0].reset();
        mPointShadowMaps[1].reset();
    }

    std::vector<const Texture *> PointShadowRenderData::depthTextures()
    {
        return { mPointShadowMaps[0]->depthTexture(), mPointShadowMaps[1]->depthTexture() };
    }

    Render::RenderTarget *PointShadowRenderData::pointShadowTarget(size_t index)
    {
        return mPointShadowMaps[index].get();
    }

    Threading::ImmediateTask<RenderFuture> PointShadowRenderData::render(RenderContext *context)
    {
        co_await mPointShadowMaps[0]->update(context);
        co_await mPointShadowMaps[1]->update(context);

        co_return {};
    }

}
}