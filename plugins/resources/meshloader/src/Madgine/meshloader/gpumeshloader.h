#pragma once

#include "Modules/threading/workgroupstorage.h"

#include "Madgine/resources/virtualresourceloader.h"

#include "gpumeshdata.h"

namespace Engine {
namespace Render {

    struct MADGINE_MESHLOADER_EXPORT GPUMeshLoader : Resources::VirtualResourceLoaderBase<GPUMeshLoader, GPUMeshData, std::list<Placeholder<0>>, Threading::WorkGroupStorage> {

        using Base = VirtualResourceLoaderBase<GPUMeshLoader, GPUMeshData, std::list<Placeholder<0>>, Threading::WorkGroupStorage>;

        struct Handle : Base::Handle {
            using Base::Handle::Handle;
            Handle(Base::Handle handle)
                : Base::Handle(std::move(handle))
            {
            }

            Threading::TaskFuture<bool> createUnnamed(MeshData mesh);
        };

        GPUMeshLoader();

        Threading::Task<void> finalize() override;

        Threading::Task<bool> loadImpl(GPUMeshData &data, const ResourceDataInfo &info);
        void unloadImpl(GPUMeshData &data);

        virtual Threading::Task<bool> generate(GPUMeshData &data, const MeshData &mesh) = 0;

        virtual void reset(GPUMeshData &data) = 0;

        Handle mQuad;
    };

}

namespace Serialize {
    template <>
    struct Operations<Render::GPUMeshLoader::Handle> : Operations<Render::GPUMeshLoader::Base::Handle> { };
}

}