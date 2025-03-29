#include "../scenerenderertoolslib.h"

#include "gridpass.h"

//#include "OpenGL/openglshaderloader.h"

#include "Madgine/meshloader/meshloader.h"

#include "Meta/math/matrix4.h"

#include "Madgine/render/camera.h"
#include "Madgine/render/rendertarget.h"
#include "Madgine/render/rendercontext.h"
#include "Madgine/render/vertex.h"

#include "Madgine/meshloader/gpumeshloader.h"


#include "Madgine/render/shadinglanguage/sl_support_begin.h"
#include "shaders/grid.sl"
#include "Madgine/render/shadinglanguage/sl_support_end.h"


namespace Engine {
namespace Tools {

    GridPass::GridPass(Render::Camera *camera, int priority)
        : mCamera(camera)
        , mPriority(priority)
    {

        mMesh.load("Plane");

        mPipeline.create({ .vs = "grid", .ps = "grid", .bufferSizes = { 0, sizeof(GridPerFrame) } });

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
            auto parameters = mPipeline->mapParameters<GridPerFrame>(1);

            parameters->vp = target->getClipSpaceMatrix() * mCamera->getViewProjectionMatrix(aspectRatio);
        }

        mPipeline->bindMesh(target, mMesh);
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