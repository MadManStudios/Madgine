#pragma once

#include "Madgine/resources/resourceloader.h"

#include "Modules/threading/workgroupstorage.h"

namespace Engine {
namespace Render {

    struct DirectX12PixelShaderLoader : Resources::ResourceLoader<DirectX12PixelShaderLoader, ReleasePtr<IDxcBlob>, std::list<Placeholder<0>>, Threading::WorkGroupStorage> {
        DirectX12PixelShaderLoader();

        struct Ptr : Base::Ptr {

            using Base::Ptr::Ptr;
            Ptr(Base::Ptr ptr)
                : Base::Ptr(std::move(ptr))
            {
            }

            Threading::TaskFuture<bool> create(const CodeGen::ShaderFile &file, DirectX12PixelShaderLoader *loader = &DirectX12PixelShaderLoader::getSingleton());
        };


        bool loadImpl(ReleasePtr<IDxcBlob> &shader, ResourceDataInfo &info);
        void unloadImpl(ReleasePtr<IDxcBlob> &shader);

        bool create(ReleasePtr<IDxcBlob> &shader, const CodeGen::ShaderFile &file);

        bool loadFromSource(ReleasePtr<IDxcBlob> &shader, std::string_view name, std::string source);

        virtual Threading::TaskQueue *loadingTaskQueue() const override;

    private:
        ReleasePtr<IDxcLibrary> mLibrary;
        ReleasePtr<IDxcCompiler3> mCompiler;
    };

}
}

REGISTER_TYPE(Engine::Render::DirectX12PixelShaderLoader)