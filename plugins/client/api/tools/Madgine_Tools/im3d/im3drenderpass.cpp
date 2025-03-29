#include "../clienttoolslib.h"

#include "im3drenderpass.h"

#include "Madgine/render/rendertarget.h"
#include "Madgine/render/rendercontext.h"

#include "im3d/im3d.h"
#include "im3d/im3d_internal.h"

#include "Madgine/render/camera.h"

#include "Madgine/render/texturedescriptor.h"

#include "Madgine/meshloader/meshdata.h"

#include "Madgine/render/shadinglanguage/sl_support_begin.h"
#include "shaders/im3d.sl"
#include "Madgine/render/shadinglanguage/sl_support_end.h"

namespace Engine {
namespace Render {

    Im3DRenderPass::Im3DRenderPass(Camera *camera, int priority)
        : mCamera(camera)
        , mPriority(priority)
    {
        mPipeline.create({ .vs = "im3d", .ps = "im3d", .bufferSizes = { sizeof(Im3DPerApplication), 0, sizeof(Im3DPerObject) } });
    }

    void Im3DRenderPass::render(RenderTarget *target, size_t iteration)
    {
        if (!mPipeline.available())
            return;

        Im3D::Im3DContext *context = Im3D::GetCurrentContext();

        target->clearDepthBuffer();

        Vector2i size = target->size();

        float aspectRatio = float(size.x) / size.y;

        {
            auto perApplication = mPipeline->mapParameters<Im3DPerApplication>(0);

            perApplication->p = target->getClipSpaceMatrix() * mCamera->getProjectionMatrix(aspectRatio);
        }

        /*for (const std::pair<Im3DNativeMesh, std::vector<Matrix4>> &p : context->mNativeMeshes)
            target->renderInstancedMesh(RenderPassFlags_NoLighting, p.first, p.second);*/

        for (std::pair<const Im3DTextureId, Im3D::Im3DContext::RenderData> &p : context->mRenderData) {

            {
                auto perObject = mPipeline->mapParameters<Im3DPerObject>(2);

                perObject->hasDistanceField = false;

                perObject->mv = mCamera->getViewMatrix();

                perObject->hasTexture = p.first != 0;
                perObject->hasDistanceField = bool(p.second.mFlags & RenderPassFlags_DistanceField);
            }

            mPipeline->bindResources(target, 2, { p.first });

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
