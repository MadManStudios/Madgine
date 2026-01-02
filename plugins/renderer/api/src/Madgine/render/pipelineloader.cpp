#include "../renderlib.h"

#include "pipelineloader.h"

#include "Modules/threading/taskqueue.h"

#include "Madgine/codegen/codegen_shader.h"
#include "Madgine/meshloader/gpumeshloader.h"

#include "Meta/keyvalue/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

#include "shadercache.h"

METATABLE_BEGIN(Engine::Render::PipelineInstance)
METATABLE_END(Engine::Render::PipelineInstance)

VIRTUALRESOURCELOADERBASE(Engine::Render::PipelineLoader)

namespace Engine {
namespace Render {

    PipelineLoader::PipelineLoader(std::string target, std::string extension)
        : VirtualResourceLoaderBase(std::vector<std::string> {})
        , mTarget(std::move(target))
        , mExtension(std::move(extension))
    {
    }

    PipelineLoader::Instance::Instance(std::unique_ptr<PipelineInstance> ptr)
        : mPtr(std::move(ptr))
    {
    }

    PipelineLoader::Instance &PipelineLoader::Instance::operator=(std::unique_ptr<PipelineInstance> ptr)
    {
        std::swap(mPtr, ptr);
        return *this;
    }

    Threading::TaskFuture<bool> PipelineLoader::Instance::create(PipelineConfiguration config, PipelineLoader *loader)
    {
        assert(!mState.valid());
        mState = loader->loadingTaskQueue()->queueTask(loader->create(*this, std::move(config)));
        return mState;
    }

    bool PipelineLoader::Instance::available() const
    {
        return mState.valid() && mState.is_ready() && mState;
    }

    void PipelineLoader::Instance::reset()
    {
        mPtr.reset();
        mState.reset();
    }

    PipelineLoader::Instance::operator PipelineInstance *() const
    {
        return mPtr.get();
    }

    PipelineInstance *PipelineLoader::Instance::operator->() const
    {
        return mPtr.get();
    }

    void PipelineInstance::renderQuad(RenderTarget *target) const
    {
        if (!GPUMeshLoader::getSingleton().mQuad)
            GPUMeshLoader::getSingleton().mQuad.load("quad");
        if (GPUMeshLoader::getSingleton().mQuad.available()) {
            bindMesh(target, GPUMeshLoader::getSingleton().mQuad);
            render(target);
        }
    }

    Threading::Task<Resources::BakeResult> PipelineLoader::bakeResources(std::vector<Filesystem::Path> &resourcesToBake, const Filesystem::Path &intermediateDir)
    {
        Resources::BakeResult result = Resources::BakeResult::SUCCESS;

        for (ShaderObjectPtr object : ShaderCache::shaderCache()) {

            Filesystem::Path p = intermediateDir.parentPath() / "shadercache" / (object->entrypoint() + mExtension);

            if (!co_await ShaderCache::generate(p, object, mTarget, ShaderType::PixelShader)) {
                result = Resources::BakeResult::UNKNOWN_ERROR;
            }
        }

        co_return result;
    }

}
}