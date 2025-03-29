#define NOMINMAX
#ifdef _WIN32
#    include <Windows.h>
#endif
#include "dxc/dxcapi.h"

#include <spirv_hlsl.hpp>

#include <vector>

#include <memory>

#include <assert.h>

#include <iostream>

#include <map>

#include <fstream>

#include <codecvt>

struct ReleaseDeleter {
    template <typename T>
    void operator()(T *ptr)
    {
        ptr->Release();
    }
};

template <typename T>
struct ReleasePtr : std::unique_ptr<T, ReleaseDeleter> {
    using std::unique_ptr<T, ReleaseDeleter>::unique_ptr;

    T **operator&()
    {
        assert(!*this);
        return reinterpret_cast<T **>(this);
    }

    T *const *operator&() const
    {
        return reinterpret_cast<T *const *>(this);
    }

    operator T *() const
    {
        return this->get();
    }
};

#define CHECK_HR(Operation)                                          \
    if (FAILED(hr)) {                                                \
        std::cerr << "Error in " #Operation ": " << hr << std::endl; \
        return -1;                                                   \
    }

extern ReleasePtr<IDxcLibrary> library;
extern ReleasePtr<IDxcCompiler3> compiler;
extern ReleasePtr<IDxcIncludeHandler> includeHandler;

static std::map<std::string, uint32_t> sSemanticLocationMappings {
    { "POSITION0", 0 },
    { "POSITION1", 1 },
    { "POSITION2", 2 },
    { "NORMAL", 3 },
    { "COLOR", 4 },
    { "TEXCOORD", 5 },
    { "BONEINDICES", 6 },
    { "WEIGHTS", 7 },
    { "INSTANCEDATA", 8 },
    { "INSTANCEDATA1", std::numeric_limits<uint32_t>::max() },
    { "INSTANCEDATA2", std::numeric_limits<uint32_t>::max() },
    { "INSTANCEDATA3", std::numeric_limits<uint32_t>::max() },
    { "INSTANCEDATA4", std::numeric_limits<uint32_t>::max() },
    { "INSTANCEDATA5", std::numeric_limits<uint32_t>::max() },
    { "INSTANCEDATA6", std::numeric_limits<uint32_t>::max() }
};

int transpileHLSL(int apilevel, const std::wstring &fileName, const std::wstring &outFolder, IDxcResult *result, bool debug, const std::vector<std::wstring> &includes)
{

    std::cout << "HLSL (DX" << apilevel << ") ... ";

    std::wstring extension = fileName.substr(fileName.rfind('.'));

    std::string shaderCode;

    if ((apilevel == 12 && debug)) {
        std::cout << "Skipping for Debugging, only preprocessing... ";

        ReleasePtr<IDxcBlobEncoding> pSource;
        HRESULT hr = library->CreateBlobFromFile(fileName.c_str(), nullptr, &pSource);
        CHECK_HR(CreateBlobFromFile);

        std::vector<LPCWSTR> arguments;

        arguments.push_back(fileName.c_str());

        arguments.push_back(L"-P");

        for (const std::wstring &include : includes) {
            arguments.push_back(L"-I");
            arguments.push_back(include.c_str());
        }

        DxcBuffer sourceBuffer;
        sourceBuffer.Ptr = pSource->GetBufferPointer();
        sourceBuffer.Size = pSource->GetBufferSize();
        sourceBuffer.Encoding = 0;

        ReleasePtr<IDxcResult> pCompileResult;
        hr = compiler->Compile(&sourceBuffer, arguments.data(), arguments.size(), includeHandler, IID_PPV_ARGS(&pCompileResult));
        CHECK_HR(Compile);

        ReleasePtr<IDxcBlobUtf8> pPrecompiled;
        hr = pCompileResult->GetOutput(DXC_OUT_HLSL, IID_PPV_ARGS(&pPrecompiled), nullptr);
        CHECK_HR(GetOutput / HLSL);

        shaderCode = { pPrecompiled->GetStringPointer(), pPrecompiled->GetStringLength() };

    } else {

        try {

            ReleasePtr<IDxcBlob> pSpirv;
            HRESULT hr = result->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&pSpirv), nullptr);
            CHECK_HR(GetOutput / Object);

            spirv_cross::CompilerHLSL hlsl { (uint32_t *)pSpirv->GetBufferPointer(), pSpirv->GetBufferSize() / 4 };
            spirv_cross::CompilerHLSL::Options options {};
            options.shader_model = 50;
            options.flatten_matrix_vertex_input_semantics = true;
            hlsl.set_hlsl_options(options);
            spirv_cross::CompilerGLSL::Options common_options {};
            common_options.relax_nan_checks = true;
            hlsl.set_common_options(common_options);

            for (const spirv_cross::VariableID &id : hlsl.get_active_interface_variables()) {
                if (hlsl.get_storage_class(id) == spv::StorageClassInput && hlsl.get_execution_model() == spv::ExecutionModelVertex) {
                    std::string name = hlsl.get_name(id);
                    std::string semantic = name.substr(name.rfind('.') + 1);
                    auto it = sSemanticLocationMappings.find(semantic);
                    if (it != sSemanticLocationMappings.end()) {
                        uint32_t location = it->second;
                        if (location != std::numeric_limits<uint32_t>::max())
                            hlsl.set_decoration(id, spv::DecorationLocation, location);
                    } else {
                        std::wcerr << fileName;
                        std::cerr << "(1,1): warning : Unsupported semantic " << semantic << " used for " << name << std::endl;
                    }
                }
            }

            for (const spirv_cross::Resource &resource : hlsl.get_shader_resources().separate_images) {
                uint32_t descriptorSpace = hlsl.get_decoration(resource.id, spv::DecorationDescriptorSet);
                uint32_t registerIndex = hlsl.get_decoration(resource.id, spv::DecorationBinding);
                hlsl.set_decoration(resource.id, spv::DecorationBinding, 16 + descriptorSpace * 4 + registerIndex);
            }

            //hlsl.build_dummy_sampler_for_combined_images();

            //hlsl.build_combined_image_samplers();

            shaderCode = hlsl.compile();

        } catch (spirv_cross::CompilerError &error) {
            std::wcerr << fileName;
            std::cerr << "(1,1): error: " << error.what() << "\n";
            return -1;
        }
    }

    auto fileNameBegin = fileName.rfind('/');
    std::wstring outputFile = outFolder + L"/" + fileName.substr(fileNameBegin + 1) + std::to_wstring(apilevel);

    std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;

    std::ofstream of { converter.to_bytes( outputFile ) };

    of << shaderCode << std::endl;

    std::cout << "Success!" << std::endl;

    return 0;
}
