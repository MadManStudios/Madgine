#include "directx12lib.h"

#include "directx12pipelineloader.h"

#include "directx12pixelshaderloader.h"

#include "util/directx12pipelineinstance.h"

#include "Meta/keyvalue/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

#include "Madgine/codegen/codegen_shader.h"

#include "directx12rendercontext.h"

VIRTUALRESOURCELOADERIMPL(Engine::Render::DirectX12PipelineLoader, Engine::Render::PipelineLoader);

namespace Engine {
namespace Render {

    DirectX12PipelineLoader::DirectX12PipelineLoader()
    {
    }

    bool DirectX12PipelineLoader::loadImpl(DirectX12Pipeline &pipeline, ResourceDataInfo &info)
    {
        throw 0;
    }

    void DirectX12PipelineLoader::unloadImpl(DirectX12Pipeline &pipeline)
    {
        pipeline.reset();
    }

    Threading::Task<bool> DirectX12PipelineLoader::create(Instance &instance, PipelineConfiguration config)
    {
        Handle pipeline;

        char buffer[256];
#if WINDOWS
        sprintf_s(buffer, "%s|%s", config.vs.data(), config.ps.data());
#else
        sprintf(buffer, "%s|%s", config.vs.data(), config.ps.data());
#endif

        if (!co_await pipeline.create(buffer, {}, [vs { std::string { config.vs } }, ps { std::string { config.ps } }](DirectX12PipelineLoader *loader, DirectX12Pipeline &pipeline, ResourceDataInfo &info) -> Threading::Task<bool> {
                pipeline.setName(info.resource()->name());
                DirectX12VertexShaderLoader::Handle vertexShader;
                if (!co_await vertexShader.load(vs)) {
                    LOG_ERROR("Failed to load VS '" << vs << "'!");
                    co_return false;
                }
                DirectX12PixelShaderLoader::Handle pixelShader;
                if (!ps.empty() && !co_await pixelShader.load(ps)) {
                    LOG_ERROR("Failed to load PS '" << ps << "'!");
                    co_return false;
                }
                co_return pipeline.link(std::move(vertexShader), std::move(pixelShader));
            }))
            co_return false;

        instance = std::make_unique<DirectX12PipelineInstanceHandle>(config, std::move(pipeline));

        co_return true;
    }

    Threading::Task<bool> DirectX12PipelineLoader::create(Instance &instance, PipelineConfiguration config, CodeGen::ShaderFile file)
    {
        assert(file.mInstances.size() == 2);

        Ptr pipeline;

        if (!co_await pipeline.create([file { std::move(file) }](DirectX12PipelineLoader *loader, DirectX12Pipeline &pipeline) -> Threading::Task<bool> {
                DirectX12VertexShaderLoader::Ptr vertexShader;
                if (!co_await vertexShader.create(file))
                    co_return false;
                DirectX12PixelShaderLoader::Ptr pixelShader;
                if (!co_await pixelShader.create(file))
                    co_return false;
                co_return pipeline.link(std::move(vertexShader), std::move(pixelShader));
            }))
            co_return false;

        instance = std::make_unique<DirectX12PipelineInstancePtr>(config, std::move(pipeline));

        co_return true;
    }

    Threading::TaskQueue *DirectX12PipelineLoader::loadingTaskQueue() const
    {
        return DirectX12RenderContext::renderQueue();
    }

}
}
