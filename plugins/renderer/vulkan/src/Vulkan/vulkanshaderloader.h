#pragma once

#include "Madgine/resources/resourceloader.h"

#include "Modules/threading/workgroupstorage.h"

#include "Madgine/render/shadercache.h"

namespace Engine {
namespace Render {

    struct VulkanShader {
        VulkanPtr<VkShaderModule, &vkDestroyShaderModule> mModule;
    };

    struct VulkanShaderLoader : Resources::ResourceLoader<VulkanShaderLoader, VulkanShader, std::list<Placeholder<0>>, Threading::WorkGroupStorage> {
        VulkanShaderLoader();

        
        struct Handle : Base::Handle {

            using Base::Handle::Handle;
            Handle(Base::Handle handle)
                : Base::Handle(std::move(handle))
            {
            }

            Threading::TaskFuture<bool> load(ShaderObjectPtr object, ShaderType type, VulkanShaderLoader *loader = &VulkanShaderLoader::getSingleton());
        };


        Threading::Task<bool> loadImpl(VulkanShader &shader, ResourceDataInfo &info);
        Threading::Task<bool> generate(VulkanShader &shader, ResourceDataInfo &info, ShaderType type, ShaderObjectPtr object = {});
        void unloadImpl(VulkanShader &shader);

        bool loadFromSource(VulkanShader &shader, std::string_view name, std::vector<unsigned char> source, ShaderType type, const Filesystem::Path &path);

        virtual Threading::TaskQueue *loadingTaskQueue() const override;

    private:
    };

}
}

REGISTER_TYPE(Engine::Render::VulkanShaderLoader)