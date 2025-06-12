#pragma once

#include "Madgine/render/renderpass.h"
#include "Madgine/meshloader/gpumeshloader.h"
#include "Madgine/render/pipelineloader.h"

namespace Engine {
namespace Tools {

    struct GridPass : Render::RenderPass {

        GridPass(Render::Camera *camera, int priority);

        void setup(Render::RenderTarget *target) override;

        virtual void render(Render::RenderTarget *target, size_t iteration) override;

        virtual int priority() const override;

        virtual std::string_view name() const override;

    private:
        Render::GPUMeshLoader::Handle mMesh;

        Render::Camera *mCamera;

        int mPriority;
    };

}
}