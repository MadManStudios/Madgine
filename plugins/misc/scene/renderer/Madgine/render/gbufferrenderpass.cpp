#include "../scenerendererlib.h"

#include "gbufferrenderpass.h"

#include "Madgine/render/camera.h"
#include "Madgine/render/rendertarget.h"

#include "camerascenerenderdata.h"
#include "gbuffer_hlsl.h"
#include "scenerenderdata.h"

namespace Engine {
namespace Render {

    GBufferRenderPass::GBufferRenderPass(SceneRenderData &renderData, Camera &camera, int priority)
        : mData(renderData.registerCamera(camera))
        , mPriority(priority)
    {
    }

    GBufferRenderPass::~GBufferRenderPass()
    {
        mData.unregister();
    }

    void GBufferRenderPass::setup(RenderTarget *target)
    {
        setupImpl(target, HLSL::gbuffer_VS, HLSL::gbuffer_PS, { sizeof(HLSL::GBufferPerApplication), 0, sizeof(HLSL::GBufferPerObject) });

        addDependency(&mData);
    }

    void GBufferRenderPass::shutdown(RenderTarget *target)
    {
        removeDependency(&mData);

        RenderPass::shutdown(target);
    }

    void GBufferRenderPass::render(RenderTarget *target, size_t iteration)
    {
        if (!mPipeline.available())
            return;

        Math::Vector2i size = target->renderSpace().mSize;
        float aspectRatio = float(size.x) / size.y;

        mPipeline->mapParameters<HLSL::GBufferPerApplication>(0)->p = target->getClipSpaceMatrix() * mData.mCamera.getProjectionMatrix(aspectRatio);

        Math::Matrix4 v = mData.getViewMatrix();
        

        for (const auto &[key, objects] : mData.get(aspectRatio)) {
            // Only the instances whose bounding box reaches into the view (skinned meshes always)
            

            {
                auto perObject = mPipeline->mapParameters<HLSL::GBufferPerObject>(2);
                perObject->hasDistanceField = false;
                perObject->hasTexture = static_cast<bool>(key.mMaterial);
            }

            mPipeline->bindResources(target, 2, key.mMaterial ? key.mMaterial : mData.mRenderData.defaultMaterial());

            {
                auto instanceData = mPipeline->mapTempBuffer<HLSL::GBufferInstanceData[]>(1, objects.size());
                std::ranges::transform(objects, instanceData.mData, [&](const EntityRenderData *o) {
                    Math::Matrix4 mv = v * o->mTransformMatrix;
                    return HLSL::GBufferInstanceData {
                        mv.Transpose(),
                        mv.Inverse(),
                        o->mDiffuseColor,
                        Math::Vector4 { o->mEmissiveColor, 0.0f }
                    };
                });
            }

            mPipeline->bindMesh(target, *key.mMesh);
            mPipeline->renderInstanced(target, objects.size());
        }
    }

    int GBufferRenderPass::priority() const
    {
        return mPriority;
    }

    size_t GBufferRenderPass::targetCount(size_t) const
    {
        return 3;
    }

    std::string_view GBufferRenderPass::name() const
    {
        return "GBuffer";
    }

    void GBufferRenderPass::debugCameras(CallableView<void(const Camera &, std::string_view)> handler) const
    {
        handler(mData.mCamera, "GBufferRenderPass");
    }

}
}
