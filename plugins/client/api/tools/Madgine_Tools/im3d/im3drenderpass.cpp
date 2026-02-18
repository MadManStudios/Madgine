#include "../clienttoolslib.h"

#include "im3drenderpass.h"

#include "Madgine/meshloader/meshdata.h"
#include "Madgine/render/camera.h"
#include "Madgine/render/rendercontext.h"
#include "Madgine/render/rendertarget.h"
#include "Madgine/render/texturedescriptor.h"

#include "im3d/im3d.h"
#include "im3d/im3d_internal.h"
#include "im3d_hlsl.h"

namespace Engine {
namespace Render {

    Im3DRenderPass::Im3DRenderPass(Im3D::Im3DContext *context, Camera *camera, int priority)
        : mContext(context)
        , mCamera(camera)
        , mPriority(priority)
    {
    }

    void Im3DRenderPass::setup(RenderTarget *target)
    {
        setupImpl(target, HLSL::im3d_VS, HLSL::im3d_PS, { sizeof(HLSL::Im3DPerApplication), 0, sizeof(HLSL::Im3DPerObject) });

        HLSL::im3d_PSResourceBlock0 block;
        mDefaultBlock = block.toResourceBlock(target->context());
    }

    void Im3DRenderPass::shutdown(RenderTarget *target)
    {
        target->context()->destroyResourceBlock(mDefaultBlock);
    }

    void Im3DRenderPass::render(RenderTarget *target, size_t iteration)
    {
        if (!mPipeline.available())
            return;

        Im3D::Im3DContext *context = Im3D::SetCurrentContext(mContext);

        target->clearDepthBuffer();

        Vector2i size = target->size();

        float aspectRatio = float(size.x) / size.y;

        {
            auto perApplication = mPipeline->mapParameters<HLSL::Im3DPerApplication>(0);

            perApplication->p = target->getClipSpaceMatrix() * mCamera->getProjectionMatrix(aspectRatio);
        }

        /*for (const std::pair<Im3DNativeMesh, std::vector<Matrix4>> &p : mContext->mNativeMeshes)
            target->renderInstancedMesh(RenderPassFlags_NoLighting, p.first, p.second);*/

        for (std::pair<const Im3DTextureId, Im3D::Im3DContext::RenderData> &p : mContext->mRenderData) {

            {
                auto perObject = mPipeline->mapParameters<HLSL::Im3DPerObject>(2);

                perObject->hasDistanceField = false;

                perObject->mv = mCamera->getViewMatrix();

                perObject->hasTexture = p.first != 0;
                perObject->hasDistanceField = bool(p.second.mFlags & RenderPassFlags_DistanceField);
            }

            ResourceBlock block { p.first };
            if (!block)
                block = mDefaultBlock;
            mPipeline->bindResources(target, 2, block);

            for (size_t i = 0; i < IM3D_MESHTYPE_COUNT; ++i) {
                if (!p.second.mVertices[i].empty()) {
                    {
                        auto vertices = mPipeline->mapVertices<Im3D::Vertex[]>(target, p.second.mVertices[i].size());
                        std::ranges::copy(p.second.mVertices[i], vertices.mData);
                    }
                    {
                        auto indices = mPipeline->mapIndices(target, p.second.mIndices[i].size());
                        std::ranges::copy(p.second.mIndices[i], indices.mData);
                    }
                    mPipeline->setGroupSize(i + 1);
                    mPipeline->render(target);
                }

                if (!p.second.mVertices2[i].empty()) {
                    {
                        auto vertices = mPipeline->mapVertices<Im3D::Vertex2[]>(target, p.second.mVertices2[i].size());
                        std::ranges::copy(p.second.mVertices2[i], vertices.mData);
                    }
                    {
                        auto indices = mPipeline->mapIndices(target, p.second.mIndices2[i].size());
                        std::ranges::copy(p.second.mIndices2[i], indices.mData);
                    }
                    mPipeline->setGroupSize(i + 1);
                    mPipeline->render(target);
                }
            }
        }

        Im3D::SetCurrentContext(context);
    }

    int Im3DRenderPass::priority() const
    {
        return mPriority;
    }

    std::string_view Im3DRenderPass::name() const
    {
        return "Im3D";
    }
}
}
