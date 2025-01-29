#pragma once

#include "Madgine/render/renderpass.h"
#include "Madgine/meshloader/gpumeshloader.h"
#include "Madgine/pipelineloader/pipelineloader.h"

namespace Engine {
namespace Tools {

    struct GridPass : Render::RenderPass {

        GridPass(Render::Camera *camera, int priority);

        virtual void render(Render::RenderTarget *target, size_t iteration) override;

        virtual int priority() const override;

        virtual std::string_view name() const override;

    private:
        Render::GPUMeshLoader::Handle mMesh;
        Render::PipelineLoader::Instance mPipeline;

        Render::Camera *mCamera;

        int mPriority;
    };

}
}