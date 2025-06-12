#include "vulkanlib.h"

#include "vulkanshaderloader.h"

#include "Meta/keyvalue/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

#include "Modules/uniquecomponent/uniquecomponent.h"

#include "Interfaces/filesystem/fsapi.h"

#include "vulkanrendercontext.h"

UNIQUECOMPONENT(Engine::Render::VulkanShaderLoader)

METATABLE_BEGIN(Engine::Render::VulkanShaderLoader)
MEMBER(mResources)
METATABLE_END(Engine::Render::VulkanShaderLoader)

METATABLE_BEGIN_BASE(Engine::Render::VulkanShaderLoader::Resource, Engine::Resources::ResourceBase)
METATABLE_END(Engine::Render::VulkanShaderLoader::Resource)

SERIALIZETABLE_BEGIN(Engine::Render::VulkanShaderLoader::Handle)
SERIALIZETABLE_END(Engine::Render::VulkanShaderLoader::Handle)

namespace Engine {
namespace Render {

    std::wstring GetLatestPixelProfile()
    {
        return L"ps_6_0";
    }

    VulkanShaderLoader::VulkanShaderLoader()
        : ResourceLoader({ ".spirv" })
    {
    }

    Threading::Task<bool> VulkanShaderLoader::loadImpl(VulkanShader &shader, ResourceDataInfo &info)
    {
        std::string_view filename = info.resource()->name();

        ShaderType type;
        if (filename.ends_with("_VS"))
            type = ShaderType::VertexShader;
        else if (filename.ends_with("_PS"))
            type = ShaderType::PixelShader;
        else
            throw 0;

        return generate(shader, info, type);
    }

    Threading::Task<bool> VulkanShaderLoader::generate(VulkanShader &shader, ResourceDataInfo &info, ShaderType type, ShaderObjectPtr object)
    {
        const Filesystem::Path &p = info.resource()->path();

        std::string entrypoint = "main";
        if (object) {
            entrypoint = object->entrypoint();
            co_await ShaderCache::generate(p, object, "-SPIRV", type == VertexShader ? "vs_6_2" : "ps_6_2");
        }

        if (!Filesystem::exists(p))
            co_return false;

        std::vector<unsigned char> source = info.resource()->readAsBlob();

        co_return loadFromSource(shader, info.resource()->path().stem(), source, type, info.resource()->path());
    }

    void VulkanShaderLoader::unloadImpl(VulkanShader &shader)
    {
        shader.mModule.reset();
    }

    bool VulkanShaderLoader::loadFromSource(VulkanShader &shader, std::string_view name, std::vector<unsigned char> source, ShaderType type, const Filesystem::Path &path)
    {
       
        VkShaderModuleCreateInfo createInfo {};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = source.size();
        createInfo.pCode = reinterpret_cast<const uint32_t *>(source.data());
        VkResult result = vkCreateShaderModule(GetDevice(), &createInfo, nullptr, &shader.mModule);
        VK_CHECK(result);

        return true;
    }

    Threading::TaskQueue *VulkanShaderLoader::loadingTaskQueue() const
    {
        return VulkanRenderContext::renderQueue();
    }

    Threading::TaskFuture<bool> VulkanShaderLoader::Handle::load(ShaderObjectPtr object, ShaderType type, VulkanShaderLoader *loader)
    {
        return Base::Handle::create(object->name(), ShaderCache::directory() / (std::string { object->metadata().mPath.stem() } + "_" + (type == VertexShader ? "vs" : "ps") + ".spirv"), [object, type](VulkanShaderLoader *loader, VulkanShader &shader, ResourceDataInfo &info) { return loader->generate(shader, info, type, object); }, loader);
    }
}
}
