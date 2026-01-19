#pragma once

#include "Madgine/meshloader/gpumeshloader.h"
#include "Madgine/resources/resourceloader.h"

namespace Engine {
namespace Render {

    struct MADGINE_RENDER_EXPORT RenderMeshLoader : Resources::VirtualResourceLoaderImpl<RenderMeshLoader, GPUMeshData, GPUMeshLoader> {
        RenderMeshLoader();

        virtual Threading::Task<bool> generate(GPUMeshData &data, const MeshData &mesh) override;

        virtual void reset(GPUMeshData &data) override;

        virtual Threading::TaskQueue *loadingTaskQueue() const override;
    };
}
}