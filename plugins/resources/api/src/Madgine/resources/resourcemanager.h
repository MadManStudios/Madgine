#pragma once

#include "Generic/systemvariable.h"

#include "Platform/filesystem/filewatcher.h"

#include "Modules/threading/madgineobject.h"
#include "Modules/threading/taskqueue.h"
#include "Modules/uniquecomponent/uniquecomponentcontainer.h"

#include "Madgine/root/rootcomponentbase.h"
#include "Madgine/root/rootcomponentcollector.h"

#include "resourceloadercollector.h"

namespace Engine {
namespace Resources {
    struct MADGINE_RESOURCES_EXPORT ResourceManager : Core::RootComponent<ResourceManager>, Threading::MadgineObject<ResourceManager> {
        static ResourceManager &getSingleton();

        ResourceManager(Core::Root &root);
        ~ResourceManager();

        virtual std::string_view key() const override;

        virtual Threading::Task<int> runTools() override;

        void registerResourceLocation(const Platform::Filesystem::Path &path, std::string_view identifier, int priority);

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

        Platform::Filesystem::Path findResourceFile(std::string_view fileName);

        Threading::Task<void> update();

        void waitForInit();

        Threading::TaskQueue *taskQueue();

        std::map<std::pair<std::string, Platform::Filesystem::Path>, std::vector<ResourceBase *>> buildResourceList();
        std::pair<std::string, Platform::Filesystem::Path> makeRelative(const Platform::Filesystem::Path &path) const;

    private:
        void updateResources(Platform::Filesystem::FileEventType event, const Platform::Filesystem::Path &path, int priority);
        void updateResources(Platform::Filesystem::FileEventType event, const Platform::Filesystem::Path &path, int priority, const std::map<std::string, std::vector<ResourceLoaderBase *>, std::less<>> &loaderByExtension);

        void updateResource(Platform::Filesystem::FileEventType event, const Platform::Filesystem::Path &path, int priority, const std::map<std::string, std::vector<ResourceLoaderBase *>, std::less<>> &loaderByExtension);

        std::map<std::string, std::vector<ResourceLoaderBase *>, std::less<>> getLoaderByExtension();

        void enumerateResources();

        Platform::Filesystem::Path getProjectPath(std::string_view name) const;

    private:
        struct SubDirCompare {
            bool operator()(const Platform::Filesystem::Path &first, const Platform::Filesystem::Path &second) const;
        };

        Platform::Filesystem::FileWatcher mFileWatcher;

        struct PathProperties {
            int mPriority;
            std::string mIdentifier;
        };
        std::map<Platform::Filesystem::Path, PathProperties, SubDirCompare> mResourcePaths;

        SystemVariable<bool, false> mEnumerated;
    };

}
}