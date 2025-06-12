#include "opengllib.h"

#include "openglpipelineloader.h"

#include "openglshaderloader.h"

#include "Meta/keyvalue/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

#include "Madgine/codegen/codegen_shader.h"

#include "openglrendercontext.h"

#include "util/openglpipelineinstance.h"

VIRTUALRESOURCELOADERIMPL(Engine::Render::OpenGLPipelineLoader, Engine::Render::PipelineLoader);

namespace Engine {
namespace Render {

    OpenGLPipelineLoader::OpenGLPipelineLoader()
    {
    }

    bool OpenGLPipelineLoader::loadImpl(OpenGLPipeline &pipeline, ResourceDataInfo &info)
    {
        throw 0;
    }

    void OpenGLPipelineLoader::unloadImpl(OpenGLPipeline &pipeline)
    {
        pipeline.reset();
    }

    Threading::Task<bool> OpenGLPipelineLoader::create(Instance &instance, PipelineConfiguration config)
    {
        std::string name = config.vs->name() + "|";
        if (config.ps)
            name += config.ps->name();

        Handle pipeline;
        if (!co_await pipeline.create(name, {}, [config](OpenGLPipelineLoader *loader, OpenGLPipeline &pipeline, ResourceDataInfo &info) -> Threading::Task<bool> {
                OpenGLShaderLoader::Handle vertexShader;
                if (!co_await vertexShader.load(config.vs, VertexShader)) {
                    LOG_ERROR("Failed to load VS '" << config.vs << "'!");
                    co_return false;
                }

                OpenGLShaderLoader::Handle pixelShader;
                if (config.ps && !co_await pixelShader.load(config.ps, PixelShader)) {
                    LOG_ERROR("Failed to load PS '" << config.ps << "'!");
                    co_return false;
                }

                if (!pipeline.link(std::move(vertexShader), std::move(pixelShader))) {
                    LOG_ERROR("Failed to link Program '" << config.vs << "|" << config.ps
                                                         << "'!");
                    co_return false;
                }

                co_return true;
            }))
            co_return false;

        instance = std::make_unique<OpenGLPipelineInstanceHandle>(config, std::move(pipeline));

        co_return true;
    }

    Threading::TaskQueue *OpenGLPipelineLoader::loadingTaskQueue() const
    {
        return OpenGLRenderContext::renderQueue();
    }

}
}
