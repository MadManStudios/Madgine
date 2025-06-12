#include "../renderlib.h"

#include "pipelineloader.h"

#include "Meta/keyvalue/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

#include "Madgine/codegen/codegen_shader.h"

#include "Madgine/meshloader/gpumeshloader.h"

#include "Modules/threading/taskqueue.h"

METATABLE_BEGIN(Engine::Render::PipelineInstance)
METATABLE_END(Engine::Render::PipelineInstance)

VIRTUALRESOURCELOADERBASE(Engine::Render::PipelineLoader)

namespace Engine {
namespace Render {

    PipelineLoader::PipelineLoader()
        : VirtualResourceLoaderBase(std::vector<std::string> {})
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

}
}