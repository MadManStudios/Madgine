#include "../scenerenderertoolslib.h"

#include "gridpass.h"

// #include "OpenGL/openglshaderloader.h"

#include "Meta/math/matrix4.h"

#include "Madgine/meshloader/gpumeshloader.h"
#include "Madgine/meshloader/meshloader.h"
#include "Madgine/render/camera.h"
#include "Madgine/render/rendercontext.h"
#include "Madgine/render/rendertarget.h"
#include "Madgine/render/vertex.h"

#include "grid_hlsl.h"

namespace Engine {
namespace Tools {

    GridPass::GridPass(Render::Camera *camera, int priority)
        : mCamera(camera)
        , mPriority(priority)
    {
        mMesh.load("Plane");
    }

    void GridPass::setup(Render::RenderTarget *target)
    {
        setupImpl(target, HLSL::grid_VS, HLSL::grid_PS, { 0, sizeof(HLSL::GridPerFrame) });        
    }

    void GridPass::render(Render::RenderTarget *target, size_t iteration)
    {
        if (!mPipeline.available())
            return;
        if (!mMesh.available())
            return;

        Vector2i size = target->size();
        float aspectRatio = float(size.x) / size.y;
        {
            auto parameters = mPipeline->mapParameters<HLSL::GridPerFrame>(1);

            parameters->vp = target->getClipSpaceMatrix() * mCamera->getViewProjectionMatrix(aspectRatio);
            parameters->cameraPos = mCamera->mPosition;
        }

        mPipeline->bindMesh(target, *mMesh);
        mPipeline->render(target);
    }

    int GridPass::priority() const
    {
        return mPriority;
    }

    std::string_view GridPass::name() const
    {
        return "Grid";
    }
}
}