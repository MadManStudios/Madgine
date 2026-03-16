#pragma once

#include "Generic/systemvariable.h"

#include "Interfaces/filesystem/filewatcher.h"

#include "Modules/threading/madgineobject.h"
#include "Modules/threading/taskqueue.h"
#include "Modules/uniquecomponent/uniquecomponentcontainer.h"

#include "Madgine/root/rootcomponentbase.h"
#include "Madgine/root/rootcomponentcollector.h"

#include "resourceloadercollector.h"

namespace Engine {
namespace Resources {
    struct MADGINE_RESOURCES_EXPORT ResourceManager : Root::RootComponent<ResourceManager>, Threading::MadgineObject<ResourceManager> {
        static ResourceManager &getSingleton();

        ResourceManager(Root::Root &root);
        ~ResourceManager();

        virtual std::string_view key() const override;

        virtual Threading::Task<int> runTools() override;

        void registerResourceLocation(const Filesystem::Path &path, std::string_view identifier, int priority);

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

        Filesystem::Path findResourceFile(std::string_view fileName);

        Threading::Task<void> update();

        void waitForInit();

        Threading::TaskQueue *taskQueue();

        std::map<std::pair<std::string, Filesystem::Path>, std::vector<ResourceBase *>> buildResourceList();
        std::pair<std::string, Filesystem::Path> makeRelative(const Filesystem::Path &path) const;

    private:
        void updateResources(Filesystem::FileEventType event, const Filesystem::Path &path, int priority);
        void updateResources(Filesystem::FileEventType event, const Filesystem::Path &path, int priority, const std::map<std::string, std::vector<ResourceLoaderBase *>, std::less<>> &loaderByExtension);

        void updateResource(Filesystem::FileEventType event, const Filesystem::Path &path, int priority, const std::map<std::string, std::vector<ResourceLoaderBase *>, std::less<>> &loaderByExtension);

        std::map<std::string, std::vector<ResourceLoaderBase *>, std::less<>> getLoaderByExtension();

        void enumerateResources();

        Filesystem::Path getProjectPath(std::string_view name) const;

    private:
        struct SubDirCompare {
            bool operator()(const Filesystem::Path &first, const Filesystem::Path &second) const;
        };

        Filesystem::FileWatcher mFileWatcher;

        struct PathProperties {
            int mPriority;
            std::string mIdentifier;
        };
        std::map<Filesystem::Path, PathProperties, SubDirCompare> mResourcePaths;

        SystemVariable<bool, false> mEnumerated;
    };

}
}