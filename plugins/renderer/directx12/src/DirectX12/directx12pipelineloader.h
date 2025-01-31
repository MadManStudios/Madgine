#pragma once

#include "Madgine/resources/resourceloader.h"

#include "Madgine/render/pipelineloader.h"

#include "util/directx12pipeline.h"

namespace Engine {
namespace Render {

    struct DirectX12PipelineInstance;

    struct MADGINE_DIRECTX12_EXPORT DirectX12PipelineLoader : Resources::VirtualResourceLoaderImpl<DirectX12PipelineLoader, DirectX12Pipeline, PipelineLoader> {
        DirectX12PipelineLoader();

        bool loadImpl(DirectX12Pipeline &program, ResourceDataInfo &info);
        void unloadImpl(DirectX12Pipeline &program);
        Threading::Task<bool> create(Instance &instance, PipelineConfiguration config) override;
        Threading::Task<bool> create(Instance &instance, PipelineConfiguration config, CodeGen::ShaderFile file) override;

        virtual Threading::TaskQueue *loadingTaskQueue() const override;
    };
}
}

REGISTER_TYPE(Engine::Render::DirectX12PipelineLoader)