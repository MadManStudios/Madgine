#include "directx12lib.h"

#include "directx12vertexshaderloader.h"

#include "Meta/keyvalue/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

#include "Interfaces/filesystem/fsapi.h"

#include "directx12rendercontext.h"

#include "Madgine/render/shadercache.h"

#include "Interfaces/process/processapi.h"

RESOURCELOADER(Engine::Render::DirectX12VertexShaderLoader);

namespace Engine {
namespace Render {

    std::wstring GetLatestVertexProfile()
    {
        return L"vs_6_0";
    }

    DirectX12VertexShaderLoader::DirectX12VertexShaderLoader()
        : ResourceLoader({ ".vs_hlsl12" })
    {
        HRESULT hr = DxcCreateInstance(CLSID_DxcLibrary, IID_PPV_ARGS(&mLibrary));
        // if(FAILED(hr)) Handle error...

        hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&mCompiler));
        // if(FAILED(hr)) Handle error
    }

    Threading::Task<bool> DirectX12VertexShaderLoader::loadImpl(ReleasePtr<IDxcBlob> &shader, ResourceDataInfo &info)
    {
        return generate(shader, info);
    }

    Threading::Task<bool> DirectX12VertexShaderLoader::generate(ReleasePtr<IDxcBlob> &shader, ResourceDataInfo &info, ShaderObjectPtr object)
    {
        const Filesystem::Path &p = info.resource()->path();

        std::string entrypoint = "main";
        if (object) {
            entrypoint = object->entrypoint();
            co_await ShaderCache::generate(p, object, "-HLSL12", "vs_6_2");
        }
     
        if (!Filesystem::exists(p))
            co_return false;

        std::string source = info.resource()->readAsText();

        co_return loadFromSource(shader, info.resource()->path().stem(), source, entrypoint);
    }

    void DirectX12VertexShaderLoader::unloadImpl(ReleasePtr<IDxcBlob> &shader)
    {
        shader.reset();
    }

    bool DirectX12VertexShaderLoader::loadFromSource(ReleasePtr<IDxcBlob> &shader, std::string_view name, std::string source, std::string entrypoint)
    {
        std::wstring profile = L"latest";
        if (profile == L"latest")
            profile = GetLatestVertexProfile();

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
            &sourceBuffer,
            arguments.data(),
            arguments.size(),
            nullptr,
            IID_PPV_ARGS(&result));
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
        pdbFile.write(static_cast<const char *>(pDebugData->GetBufferPointer()), pDebugData->GetBufferSize());

        return true;
    }

    Threading::TaskQueue *DirectX12VertexShaderLoader::loadingTaskQueue() const
    {
        return DirectX12RenderContext::renderQueue();
    }

    Threading::TaskFuture<bool> DirectX12VertexShaderLoader::Handle::load(ShaderObjectPtr object, DirectX12VertexShaderLoader *loader)
    {
        return Base::Handle::create(object->name(), ShaderCache::directory() / (std::string { object->metadata().mPath.stem() } + ".vs_hlsl12"), [object](DirectX12VertexShaderLoader *loader, ReleasePtr<IDxcBlob> &shader, ResourceDataInfo &info) { return loader->generate(shader, info, object); }, loader);
    }
}
}
