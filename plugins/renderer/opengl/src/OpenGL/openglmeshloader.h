#pragma once

#include "Madgine/resources/resourceloader.h"

#include "Madgine/meshloader/gpumeshloader.h"

#include "openglmeshdata.h"

namespace Engine {
namespace Render {

    struct MADGINE_OPENGL_EXPORT OpenGLMeshLoader : Resources::VirtualResourceLoaderImpl<OpenGLMeshLoader, OpenGLMeshData, GPUMeshLoader> {
        OpenGLMeshLoader();

        virtual Threading::Task<bool> generate(GPUMeshData &data, const MeshData &mesh) override;

        virtual void reset(GPUMeshData &data) override;

        virtual Threading::TaskQueue *loadingTaskQueue() const override;
    };
}
}

REGISTER_TYPE(Engine::Render::OpenGLMeshLoader)