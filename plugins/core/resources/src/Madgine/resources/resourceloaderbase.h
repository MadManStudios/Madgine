#pragma once

#include "Meta/keyvalue/virtualscope.h"
#include "Modules/threading/task.h"
#include "Modules/threading/madgineobject.h"
#include "Generic/genericresult.h"

namespace Engine {
namespace Resources {

    ENUM_BASE(BakeResult, GenericResult,
        NOTHING_TO_DO);

    struct ResourceLoaderSettings {
        bool mAutoLoad = false;
        bool mAutoReload = true;
        bool mInplaceReload = false;
    };

    struct MADGINE_RESOURCES_EXPORT ResourceLoaderBase : VirtualScopeBase<>, Threading::MadgineObject<ResourceLoaderBase>{

        using Resource = ResourceBase;

        ResourceLoaderBase(std::vector<std::string> &&extensions, const ResourceLoaderSettings &settings = {});
        ResourceLoaderBase(const ResourceLoaderBase &) = delete;
        virtual ~ResourceLoaderBase() = default;

        ResourceLoaderBase &operator=(const ResourceLoaderBase &) = delete;

        virtual Threading::Task<bool> init();
        virtual Threading::Task<void> finalize();

        virtual Threading::Task<BakeResult> bakeResources(std::vector<Filesystem::Path> &resources, const Filesystem::Path &intermediateDir);
        
        virtual Threading::TaskQueue *loadingTaskQueue() const;
        
        const std::vector<std::string> &fileExtensions() const;

        size_t extensionIndex(std::string_view ext) const;


        //Implemented by ResourceManager template class
        virtual std::pair<ResourceBase *, bool> addResource(const Filesystem::Path &path, std::string_view name = {}) = 0;
        virtual void updateResourceData(ResourceBase *res) = 0;

        template <typename T>
        void resourceAdded(T *res)
        {
            if (mSettings.mAutoLoad) {
                res->loadData().info()->setPersistent(true);
            }
        }

        virtual std::vector<std::pair<std::string_view, ScopePtr>> typedResources() = 0;
        virtual std::vector<const MetaTable *> resourceTypes() const = 0;
        virtual std::vector<ResourceBase *> resources() = 0;         

    protected:
        std::vector<std::string> mExtensions;

        ResourceLoaderSettings mSettings;
    };

}
}

REGISTER_TYPE(Engine::Resources::ResourceLoaderBase)