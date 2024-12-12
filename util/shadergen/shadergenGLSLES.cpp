#define NOMINMAX
#ifdef _WIN32
#    include <Windows.h>
#endif
#include "dxc/dxcapi.h"

#include <spirv_glsl.hpp>

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

int transpileGLSLES(const std::wstring &fileName, const std::wstring &outFolder, IDxcResult *result)
{
    std::cout << "GLSLES... ";

    std::string shaderCode;

    try {

        ReleasePtr<IDxcBlob> pSpirv;
        HRESULT hr = result->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&pSpirv), nullptr);
        CHECK_HR(GetOutput / Object);

        spirv_cross::CompilerGLSL glsl { (uint32_t *)pSpirv->GetBufferPointer(), pSpirv->GetBufferSize() / 4 };
        spirv_cross::CompilerGLSL::Options options {};
        options.relax_nan_checks = true;
        options.es = true;
        options.version = 300;
        glsl.set_common_options(options);

        spirv_cross::ShaderResources resources = glsl.get_shader_resources();

        std::map<spirv_cross::ID, std::pair<std::string, uint32_t>> imageData;

        for (auto &resource : resources.separate_images) {
            imageData[resource.id] = { glsl.get_name(resource.id), 4 * (glsl.get_decoration(resource.id, spv::DecorationDescriptorSet) - 1) + glsl.get_decoration(resource.id, spv::DecorationBinding) };
        }

        glsl.build_dummy_sampler_for_combined_images();

        glsl.build_combined_image_samplers();

        auto &mappings = glsl.get_combined_image_samplers();
        std::map<spirv_cross::ID, spirv_cross::ID> map;
        for (auto &mapping : mappings) {
            map[mapping.combined_id] = mapping.image_id;
        }

        resources = glsl.get_shader_resources();

        for (auto &resource : resources.sampled_images) {
            auto &data = imageData[map[resource.id]];
            glsl.set_name(resource.id, "texture" + std::to_string(data.second));
            glsl.set_decoration(resource.id, spv::DecorationBinding, data.second);
        }

        glsl.add_header_line("#define readonly ");
        glsl.add_header_line("#define buffer uniform");
        glsl.add_header_line("#define std430 std140");
        for (auto &resource : resources.storage_buffers) {
            uint32_t set = glsl.get_decoration(resource.id, spv::DecorationDescriptorSet);
            glsl.set_name(resource.base_type_id, "buffer" + std::to_string(4 + (set - 1) + glsl.get_decoration(resource.id, spv::DecorationBinding)));
        }

        for (auto& resource : resources.uniform_buffers) {            
            //glsl.set_name(resource.id, "buffer" + std::to_string(glsl.get_decoration(resource.id, spv::DecorationBinding)));
            glsl.set_name(resource.base_type_id, "buffer" + std::to_string(glsl.get_decoration(resource.id, spv::DecorationBinding)));
        }

        for (const spirv_cross::VariableID &id : glsl.get_active_interface_variables()) {
            if (glsl.get_storage_class(id) == spv::StorageClassInput && glsl.get_execution_model() == spv::ExecutionModelVertex) {
                std::string name = glsl.get_name(id);
                std::string semantic = name.substr(name.rfind('.') + 1);
                auto it = sSemanticLocationMappings.find(semantic);
                if (it != sSemanticLocationMappings.end()) {
                    uint32_t location = it->second;
                    if (location != std::numeric_limits<uint32_t>::max())
                        glsl.set_decoration(id, spv::DecorationLocation, location);
                } else {
                    std::wcerr << fileName;
                    std::cerr << "(1,1): warning : Unsupported semantic " << semantic << " used for " << name << std::endl;
                }
            }
        }

        if (glsl.get_execution_model() == spv::ExecutionModel::ExecutionModelFragment) {
            for (auto &resource : resources.stage_inputs) {
                std::string name = glsl.get_name(resource.id);
                name.replace(0, 2, "out");
                glsl.set_name(resource.id, name);
            }
        }

        shaderCode = glsl.compile();

        auto it = shaderCode.find("[]");
        while (it != std::string::npos) {
            shaderCode.replace(it, 2, "[10]");
            it = shaderCode.find("[]");
        }
    } catch (spirv_cross::CompilerError &error) {
        std::cout << std::endl;
        std::wcerr << fileName << "(1,1): error: " << error.what()
                   << "\n";
        return -1;
    }

    auto extIt = fileName.rfind('.');
    std::wstring extension = L"_" + fileName.substr(extIt + 1, 2) + L".glsl_es";

    auto fileNameBegin = fileName.rfind('/');
    std::wstring outputFile = outFolder + L"/" + (fileName.substr(fileNameBegin + 1, extIt - fileNameBegin - 1) + extension);

    std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;

    std::ofstream of { converter.to_bytes(outputFile) };

    of << shaderCode << std::endl;

    std::cout << "Success!" << std::endl;

    return 0;
}
