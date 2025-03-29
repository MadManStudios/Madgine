#pragma once

#include "Madgine/resources/resourceloader.h"

#include "Madgine/meshloader/gpumeshloader.h"

#include "directx12meshdata.h"

namespace Engine {
namespace Render {

    struct MADGINE_DIRECTX12_EXPORT DirectX12MeshLoader : Resources::VirtualResourceLoaderImpl<DirectX12MeshLoader, DirectX12MeshData, GPUMeshLoader> {
        DirectX12MeshLoader();

        virtual Threading::Task<bool> generate(GPUMeshData &data, const MeshData &mesh) override;

        virtual void reset(GPUMeshData &data) override;

        virtual Threading::TaskQueue *loadingTaskQueue() const override;
    };
}
}

REGISTER_TYPE(Engine::Render::DirectX12MeshLoader)