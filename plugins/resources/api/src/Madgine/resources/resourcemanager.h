#pragma once

#include "Modules/uniquecomponent/uniquecomponentcontainer.h"
#include "Interfaces/filesystem/filewatcher.h"
#include "resourceloadercollector.h"
#include "Modules/threading/taskqueue.h"
#include "Generic/systemvariable.h"

#include "Modules/threading/madgineobject.h"

#include "Madgine/root/rootcomponentcollector.h"
#include "Madgine/root/rootcomponentbase.h"

namespace Engine {
namespace Resources {
    struct MADGINE_RESOURCES_EXPORT ResourceManager : Root::RootComponent<ResourceManager>, Threading::MadgineObject<ResourceManager>
    {
        static ResourceManager &getSingleton();

        ResourceManager(Root::Root &root);
        ~ResourceManager();

        virtual std::string_view key() const override;

        virtual Threading::Task<int> runTools() override;

        void registerResourceLocation(const Filesystem::Path &path, int priority);

        template <typename Loader>
        typename Loader::Resource *getResource(const std::string &name)
        {
            return mCollector.get<Loader>().get(name);
        }

        template <typename Loader>
        Loader &get()
        {
            return mCollector.get<Loader>();
        }

        ResourceLoaderBase &get(size_t i)
        {
            return mCollector.get(i);
        }

        Threading::Task<bool> init();
        Threading::Task<void> finalize();

        ResourceLoaderContainer<std::vector<Placeholder<0>>> mCollector;

        Filesystem::Path findResourceFile(const std::string &fileName);

        Threading::Task<void> update();

        void waitForInit();

        Threading::TaskQueue *taskQueue();

        std::map<Filesystem::Path, std::vector<ResourceBase*>> buildResourceList();

    private:
        void updateResources(Filesystem::FileEventType event, const Filesystem::Path &path, int priority);
        void updateResources(Filesystem::FileEventType event, const Filesystem::Path &path, int priority, const std::map<std::string, std::vector<ResourceLoaderBase *>, std::less<>> &loaderByExtension);

        void updateResource(Filesystem::FileEventType event, const Filesystem::Path &path, int priority, const std::map<std::string, std::vector<ResourceLoaderBase *>, std::less<>> &loaderByExtension);

        std::map<std::string, std::vector<ResourceLoaderBase *>, std::less<>> getLoaderByExtension();

        void enumerateResources();

    private:
        struct SubDirCompare {
            bool operator()(const Filesystem::Path &first, const Filesystem::Path &second) const;
        };

        Filesystem::FileWatcher mFileWatcher;

        std::map<Filesystem::Path, int, SubDirCompare> mResourcePaths;

        SystemVariable<bool, false> mEnumerated;
    };

}
}

REGISTER_TYPE(Engine::Resources::ResourceManager)