#pragma once

#include "Madgine/resources/resourceloader.h"

#include "Modules/threading/workgroupstorage.h"

#include "Madgine/render/vertexformat.h"

#include "Madgine/render/shadercache.h"

namespace Engine {
namespace Render {

    struct DirectX12VertexShaderLoader : Resources::ResourceLoader<DirectX12VertexShaderLoader, ReleasePtr<IDxcBlob>, std::list<Placeholder<0>>, Threading::WorkGroupStorage> {
        DirectX12VertexShaderLoader();

        struct Handle : Base::Handle {

            using Base::Handle::Handle;
            Handle(Base::Handle handle)
                : Base::Handle(std::move(handle))
            {
            }

            Threading::TaskFuture<bool> load(ShaderObjectPtr object, DirectX12VertexShaderLoader *loader = &DirectX12VertexShaderLoader::getSingleton());
        };

        Threading::Task<bool> loadImpl(ReleasePtr<IDxcBlob> &shader, ResourceDataInfo &info);
        Threading::Task<bool> generate(ReleasePtr<IDxcBlob> &shader, ResourceDataInfo &info, ShaderObjectPtr object = {});
        void unloadImpl(ReleasePtr<IDxcBlob> &shader);

        bool loadFromSource(ReleasePtr<IDxcBlob> &shader, std::string_view name, std::string source, std::string entrypoint);

        virtual Threading::TaskQueue *loadingTaskQueue() const override;

    private:
        ReleasePtr<IDxcLibrary> mLibrary;
        ReleasePtr<IDxcCompiler3> mCompiler;
    };

}
}

REGISTER_TYPE(Engine::Render::DirectX12VertexShaderLoader)