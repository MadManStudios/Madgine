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

        char buffer[256];
#if WINDOWS
        sprintf_s(buffer, "%s|%s", config.vs.data(), config.ps.data());
#else
        sprintf(buffer, "%s|%s", config.vs.data(), config.ps.data());
#endif

        Handle pipeline;
        if (!co_await pipeline.create(buffer, {}, [vs { std::string { config.vs } }, ps { std::string { config.ps } }](OpenGLPipelineLoader *loader, OpenGLPipeline &pipeline, ResourceDataInfo &info) -> Threading::Task<bool> {
                OpenGLShaderLoader::Handle vertexShader;
                if (!co_await vertexShader.load(vs, VertexShader)) {
                    LOG_ERROR("Failed to load VS '" << vs << "'!");
                    co_return false;
                }

                OpenGLShaderLoader::Handle pixelShader;
                if (!ps.empty() && !co_await pixelShader.load(ps, PixelShader)) {
                    LOG_ERROR("Failed to load PS '" << ps << "'!");
                    co_return false;
                }

                if (!pipeline.link(std::move(vertexShader), std::move(pixelShader))) {
                    LOG_ERROR("Failed to link Program '" << vs << "|" << ps
                                                         << "'!");
                    co_return false;
                }

                co_return true;
            }))
            co_return false;

        instance = std::make_unique<OpenGLPipelineInstanceHandle>(config, std::move(pipeline));

        co_return true;
    }

    Threading::Task<bool> OpenGLPipelineLoader::create(Instance &instance, PipelineConfiguration config, CodeGen::ShaderFile file)
    {
        assert(file.mInstances.size() == 2);

        Ptr pipeline;
        if (!co_await pipeline.create([&](OpenGLPipelineLoader *loader, OpenGLPipeline &pipeline) -> Threading::Task<bool> {
                OpenGLShaderLoader::Ptr vertexShader;
                if (!co_await vertexShader.create(file, VertexShader)) {
                    LOG_ERROR("Failed to load VS '" << config.vs << "'!");
                    co_return false;
                }

                OpenGLShaderLoader::Ptr pixelShader;
                if (!co_await pixelShader.create(file, PixelShader) && pixelShader) {
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

        instance = std::make_unique<OpenGLPipelineInstancePtr>(config, std::move(pipeline));

        co_return true;
    }

    Threading::TaskQueue *OpenGLPipelineLoader::loadingTaskQueue() const
    {
        return OpenGLRenderContext::renderQueue();
    }

}
}
