#pragma once

#include "Madgine/resources/resourceloader.h"

#include "Modules/threading/workgroupstorage.h"

namespace Engine {
namespace Render {

    struct VulkanShaderLoader : Resources::ResourceLoader<VulkanShaderLoader, VulkanPtr<VkShaderModule, &vkDestroyShaderModule>, std::list<Placeholder<0>>, Threading::WorkGroupStorage> {
        VulkanShaderLoader();

        struct Handle : Base::Handle {

            using Base::Handle::Handle;
            Handle(Base::Handle handle)
                : Base::Handle(std::move(handle))
            {
            }

            Threading::TaskFuture<bool> load(std::string_view name, ShaderType type, VulkanShaderLoader *loader = &VulkanShaderLoader::getSingleton());
        };

        struct Ptr : Base::Ptr {

            using Base::Ptr::Ptr;
            Ptr(Base::Ptr ptr)
                : Base::Ptr(std::move(ptr))
            {
            }

            Threading::TaskFuture<bool> create(const CodeGen::ShaderFile &file, ShaderType type, VulkanShaderLoader *loader = &VulkanShaderLoader::getSingleton());
        };

        bool loadImpl(VulkanPtr<VkShaderModule, &vkDestroyShaderModule> &shader, ResourceDataInfo &info);
        void unloadImpl(VulkanPtr<VkShaderModule, &vkDestroyShaderModule> &shader);

        bool create(VulkanPtr<VkShaderModule, &vkDestroyShaderModule> &shader, const CodeGen::ShaderFile &file, ShaderType type);

        bool loadFromSource(VulkanPtr<VkShaderModule, &vkDestroyShaderModule> &shader, std::string_view name, std::vector<unsigned char> source, ShaderType type, const Filesystem::Path &path);

        virtual Threading::TaskQueue *loadingTaskQueue() const override;

    private:
    };

}
}

REGISTER_TYPE(Engine::Render::VulkanShaderLoader)