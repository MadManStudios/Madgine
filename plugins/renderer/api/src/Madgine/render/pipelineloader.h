#pragma once

#include "Madgine/resources/virtualresourceloader.h"
#include "Modules/threading/workgroupstorage.h"

#include "Madgine/render/vertexformat.h"

#include "pipeline.h"
#include "pipelineinstance.h"

namespace Engine {
namespace Render {


    struct MADGINE_RENDER_EXPORT PipelineLoader : Resources::VirtualResourceLoaderBase<PipelineLoader, Pipeline, std::list<Placeholder<0>>, Threading::WorkGroupStorage> {

        using Base = Resources::VirtualResourceLoaderBase<PipelineLoader, Pipeline, std::list<Placeholder<0>>, Threading::WorkGroupStorage>;

        struct MADGINE_RENDER_EXPORT Instance {

            Instance() = default;
            Instance(std::unique_ptr<PipelineInstance> ptr);

            Instance &operator=(std::unique_ptr<PipelineInstance> ptr);

            Threading::TaskFuture<bool> create(PipelineConfiguration config, PipelineLoader *loader = &PipelineLoader::getSingleton());


            void reset();

            bool available() const;

            operator PipelineInstance *() const;
            PipelineInstance *operator->() const;

        private:
            std::unique_ptr<PipelineInstance> mPtr;
            Threading::TaskFuture<bool> mState;
        };

        PipelineLoader();

        virtual Threading::Task<bool> create(Instance &instance, PipelineConfiguration config) = 0;
    };

}
}

REGISTER_TYPE(Engine::Render::PipelineLoader)