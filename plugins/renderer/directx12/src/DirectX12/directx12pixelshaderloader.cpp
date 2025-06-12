#include "directx12lib.h"

#include "directx12pixelshaderloader.h"

#include "Meta/keyvalue/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

#include "Interfaces/filesystem/fsapi.h"

#include "directx12rendercontext.h"

RESOURCELOADER(Engine::Render::DirectX12PixelShaderLoader);

namespace Engine {
namespace Render {

    std::wstring GetLatestPixelProfile()
    {
        return L"ps_6_0";
    }

    DirectX12PixelShaderLoader::DirectX12PixelShaderLoader()
        : ResourceLoader({ ".ps_hlsl12" })
    {
        HRESULT hr = DxcCreateInstance(CLSID_DxcLibrary, IID_PPV_ARGS(&mLibrary));
        //if(FAILED(hr)) Handle error...

        hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&mCompiler));
        //if(FAILED(hr)) Handle error
    }

    Threading::Task<bool> DirectX12PixelShaderLoader::loadImpl(ReleasePtr<IDxcBlob> &shader, ResourceDataInfo &info)
    {
        return generate(shader, info);
    }

    Threading::Task<bool> DirectX12PixelShaderLoader::generate(ReleasePtr<IDxcBlob> &shader, ResourceDataInfo &info, ShaderObjectPtr object)
    {
        const Filesystem::Path &p = info.resource()->path();

        std::string entrypoint = "main";
        if (object) {
            entrypoint = object->entrypoint();
            co_await ShaderCache::generate(p, object, "-HLSL12", "ps_6_2");
        }

        if (!Filesystem::exists(p))
            co_return false;

        std::string source = info.resource()->readAsText();

        co_return loadFromSource(shader, info.resource()->path().stem(), source, entrypoint);
    }

    void DirectX12PixelShaderLoader::unloadImpl(ReleasePtr<IDxcBlob> &shader)
    {
        shader.reset();
    }

    bool DirectX12PixelShaderLoader::loadFromSource(ReleasePtr<IDxcBlob> &shader, std::string_view name, std::string source, std::string entrypoint)
    {
        std::wstring profile = L"latest";
        if (profile == L"latest")
            profile = GetLatestPixelProfile();

        DxcBuffer sourceBuffer;
        sourceBuffer.Ptr = source.c_str();
        sourceBuffer.Size = source.size();
        sourceBuffer.Encoding = CP_UTF8;

        std::vector<LPCWSTR> arguments;
        std::wstring wentrypoint = StringUtil::toWString(entrypoint);
        arguments.push_back(L"-E");
        arguments.push_back(wentrypoint.c_str());

        arguments.push_back(L"-T");
        arguments.push_back(profile.c_str());

        arguments.push_back(L"-HV");
        arguments.push_back(L"2021");

        arguments.push_back(L"/Zi");

        ReleasePtr<IDxcResult> result;
        HRESULT hr = mCompiler->Compile(
            &sourceBuffer, // pSource
            arguments.data(), // pSourceName
            arguments.size(), // pEntryPoint
            nullptr,
            IID_PPV_ARGS(&result)); // ppResult
        if (SUCCEEDED(hr))
            result->GetStatus(&hr);
        if (FAILED(hr)) {
            LOG_FATAL("Loading of Shader '" << name << "' failed:");

            if (result) {
                ReleasePtr<IDxcBlobUtf8> pErrorBlob;
                hr = result->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&pErrorBlob), nullptr);
                if (SUCCEEDED(hr) && pErrorBlob) {
                    LOG_FATAL((char *)pErrorBlob->GetBufferPointer());
                }
            }
            return false;
        }
        result->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shader), nullptr);

        ReleasePtr<IDxcBlob> pDebugData;
        ReleasePtr<IDxcBlobUtf16> pDebugDataPath;
        result->GetOutput(DXC_OUT_PDB, IID_PPV_ARGS(&pDebugData), &pDebugDataPath);

        std::ofstream pdbFile { BINARY_DIR "/bin/" + StringUtil::fromWString(pDebugDataPath->GetStringPointer()), std::ios::out | std::ios::binary };
        pdbFile.write(static_cast<const char*>(pDebugData->GetBufferPointer()), pDebugData->GetBufferSize());

        return true;
    }

    Threading::TaskQueue *DirectX12PixelShaderLoader::loadingTaskQueue() const
    {
        return DirectX12RenderContext::renderQueue();
    }

    Threading::TaskFuture<bool> DirectX12PixelShaderLoader::Handle::load(ShaderObjectPtr object, DirectX12PixelShaderLoader *loader)
    {
        return Base::Handle::create(object->name(), ShaderCache::directory() / (std::string { object->metadata().mPath.stem() } + ".ps_hlsl12"), [object](DirectX12PixelShaderLoader *loader, ReleasePtr<IDxcBlob> &shader, ResourceDataInfo &info) { return loader->generate(shader, info, object); }, loader);
    }
}
}
