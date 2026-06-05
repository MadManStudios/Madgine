#pragma once

#include "Modules/threading/workgroupstorage.h"

#include "Madgine/resources/resourceloader.h"

#include "Modules/threading/workgroupstorage.h"

#include "Madgine/render/shadercache.h"

namespace Engine {
namespace Render {

    struct DirectX12PixelShaderLoader : Resources::ResourceLoader<DirectX12PixelShaderLoader, Platform::ReleasePtr<IDxcBlob>, std::list<Placeholder<0>>, Threading::WorkGroupStorage> {
        DirectX12PixelShaderLoader();

        struct Handle : Base::Handle {

            using Base::Handle::Handle;
            Handle(Base::Handle handle)
                : Base::Handle(std::move(handle))
            {
            }

            Threading::TaskFuture<bool> load(ShaderObjectPtr object, DirectX12PixelShaderLoader *loader = &DirectX12PixelShaderLoader::getSingleton());
        };


        Threading::Task<bool> loadImpl(Platform::ReleasePtr<IDxcBlob> &shader, ResourceDataInfo &info);
        Threading::Task<bool> generate(Platform::ReleasePtr<IDxcBlob> &shader, ResourceDataInfo &info, ShaderObjectPtr object = {});
        void unloadImpl(Platform::ReleasePtr<IDxcBlob> &shader);

        bool loadFromSource(Platform::ReleasePtr<IDxcBlob> &shader, std::string_view name, std::string source, std::string entrypoint);

        virtual Threading::TaskQueue *loadingTaskQueue() const override;

    private:
        Platform::ReleasePtr<IDxcLibrary> mLibrary;
        Platform::ReleasePtr<IDxcCompiler3> mCompiler;
    };

}
}