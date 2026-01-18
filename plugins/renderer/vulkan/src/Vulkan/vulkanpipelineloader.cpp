#include "vulkanlib.h"

#include "vulkanpipelineloader.h"

#include "Modules/uniquecomponent/uniquecomponent.h"

#include "Madgine/codegen/codegen_shader.h"

#include "Meta/keyvalue/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

#include "util/vulkanpipelineinstance.h"
#include "vulkanrendercontext.h"

VIRTUALRESOURCELOADERIMPL(Engine::Render::VulkanPipelineLoader, Engine::Render::PipelineLoader);

namespace Engine {
namespace Render {

    VulkanPipelineLoader::VulkanPipelineLoader()
    {
    }

    bool VulkanPipelineLoader::loadImpl(VulkanPipeline &pipeline, ResourceDataInfo &info)
    {
        throw 0;
    }

    void VulkanPipelineLoader::unloadImpl(VulkanPipeline &pipeline)
    {
        pipeline.reset();
    }

    Threading::Task<bool> VulkanPipelineLoader::create(Instance &instance, PipelineConfiguration config)
    {
        std::string name = config.vs->name() + "|";
        if (config.ps)
            name += config.ps->name();

        Handle pipeline;
        if (!co_await pipeline.create(name, {}, [config](VulkanPipelineLoader *loader, VulkanPipeline &pipeline, ResourceDataInfo &info) -> Threading::Task<bool> {
                PipelineSignature signature = config.vs->signature();
                VulkanShaderLoader::Handle vertexShader;
                if (!co_await vertexShader.load(config.vs, VertexShader)) {
                    LOG_ERROR("Failed to load VS '" << config.vs << "'!");
                    co_return false;
                }
                if (config.ps) {
                    signature = signature.merge(config.ps->signature());
                }
                VulkanShaderLoader::Handle pixelShader;
                if (config.ps && !co_await pixelShader.load(config.ps, PixelShader)) {
                    LOG_ERROR("Failed to load PS '" << config.ps << "'!");
                    co_return false;
                }
                co_return pipeline.link(signature, std::move(vertexShader), config.vs->entrypoint(), std::move(pixelShader), config.ps ? config.ps->entrypoint() : "");
            }))
            co_return false;

        instance = std::make_unique<VulkanPipelineInstanceHandle>(config, std::move(pipeline));

        co_return true;
    }

    Threading::TaskQueue *VulkanPipelineLoader::loadingTaskQueue() const
    {
        return VulkanRenderContext::renderQueue();
    }

    std::string_view VulkanPipelineLoader::extensionForTarget(std::string_view target, ShaderType type)
    {
        return ".spirv";
    }

}
}
