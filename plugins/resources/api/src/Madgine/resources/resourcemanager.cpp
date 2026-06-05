#include "../resourceslib.h"

#include "resourcemanager.h"

#include "Platform/filesystem/fsapi.h"

#include "Modules/plugins/plugin.h"
#include "Modules/plugins/pluginmanager.h"
#include "Modules/plugins/pluginsection.h"
#include "Modules/threading/awaitables/awaitabletimepoint.h"
#include "Modules/uniquecomponent/uniquecomponentcollector.h"

#include "Madgine/cli/parameter.h"
#include "Madgine/root/root.h"

#include "Meta/reflect/metatable_impl.h"

#include "resourcebase.h"
#include "resourceloaderbase.h"

UNIQUECOMPONENT(Engine::Resources::ResourceManager)

METATABLE_BEGIN(Engine::Resources::ResourceManager)
    MEMBER(mCollector)
METATABLE_END(Engine::Resources::ResourceManager)

namespace Engine {
namespace Resources {

    Core::Parameter<Platform::Filesystem::Path> exportResources { { "--export-resources", "-er" }, "", "If set, the resource manager will write all available resources to the specified list file." };
    Core::Parameter<Platform::Filesystem::Path> bakeResources { { "--bake" }, "", "If set, all resources listed in the specified list file will be baked." };
    Core::Parameter<Platform::Filesystem::Path> bakeOutputList { { "--bake-output-list" }, "", "If set, all baked resources will be written to a list file at the specified location." };

#if ENABLE_PLUGINS
    Core::Parameter<Platform::Filesystem::Path> sourceDirPath { { "--source-dir" }, SOURCE_DIR, "Set source root folder." };
#endif

    static ResourceManager *sSingleton = nullptr;

    ResourceManager &ResourceManager::getSingleton()
    {
        assert(sSingleton);
        return *sSingleton;
    }

    ResourceManager::ResourceManager(Core::Root &root)
        : RootComponent(root)
    {
        assert(!sSingleton);
        sSingleton = this;

        root.taskQueue()->addSetupSteps(
            [this]() { return callInit(); },
            [this]() { return callFinalize(); });
    }

    ResourceManager::~ResourceManager()
    {
    }

    std::string_view ResourceManager::key() const
    {
        return "ResourceManager";
    }

    Threading::Task<int> ResourceManager::runTools()
    {
        if (!exportResources->empty()) {

            std::map<std::pair<std::string, Platform::Filesystem::Path>, std::vector<ResourceBase *>> resourceList = buildResourceList();
            std::ofstream out { *exportResources };
            if (!out) {
                LOG_ERROR("Error opening for writing: " << *exportResources);
                co_return -1;
            }
            for (const auto &[path, resources] : resourceList) {
                out << path.first << ":" << path.second << "\n";
            }
        }

        if (bakeResources->empty() != bakeOutputList->empty()) {
            LOG_ERROR("Both baking options need to be specified or none of them!");
            co_return -1;
        }

        if (!bakeResources->empty()) {
            std::ifstream list { *bakeResources };
            if (!list) {
                LOG_ERROR("Error opening for reading: " << *bakeResources);
                co_return -1;
            }
            LOG("Baking resources in " << bakeResources << ".");
            std::vector<Platform::Filesystem::Path> resourcesToBake;
            std::string line;
            while (std::getline(list, line)) {
                line = StringUtil::trim(line);
                auto it = line.find(':');
                std::string project;
                if (it != std::string::npos) {
                    project = line.substr(0, it);
                    Platform::Filesystem::Path path = getProjectPath(project);
                    if (path.empty()) {
                        LOG_WARNING("Project " << project << " not found for resource " << line << ", skipping.");
                        continue;
                    }
                    resourcesToBake.emplace_back(path / line.substr(it + 1));
                } else {
                    resourcesToBake.emplace_back(line);
                }
            }

            for (const std::unique_ptr<ResourceLoaderBase> &loader : mCollector) {
                BakeResult result = co_await loader->bakeResources(resourcesToBake, bakeOutputList->parentPath());
                if (result == BakeResult::SUCCESS) {
                } else if (result != BakeResult::NOTHING_TO_DO) {
                    LOG_ERROR("Baking failed!");
                    co_return -1;
                }
            }

            LOG("Writing resource list for baked Resources to " << bakeOutputList);
            std::ofstream out { *bakeOutputList };
            if (!out) {
                LOG_ERROR("Error opening for writing: " << *bakeOutputList);
                co_return -1;
            }
            for (const Platform::Filesystem::Path &resource : resourcesToBake) {
                const auto &[project, path] = makeRelative(resource);
                if (project.empty()) {
                    LOG_WARNING("Resource " << resource << " is not in a registered resource location, skipping writing it to the output list.");
                    continue;
                }
                out << project << ":" << path << "\n";
            }
        }
        co_return 0;
    }

    void ResourceManager::registerResourceLocation(const Platform::Filesystem::Path &path, std::string_view identifier, int priority)
    {
        Platform::Filesystem::Path absolutePath = path.absolute();

        if (!exists(absolutePath))
            return;

        LOG("Registering resource location: " << path << " with identifier: " << identifier);

        auto [it, b] = mResourcePaths.try_emplace(absolutePath, PathProperties { priority, std::string { identifier } });
        assert(b);
        if (b) {
            mFileWatcher.addWatch(absolutePath);

            if (mEnumerated) {
                updateResources(Platform::Filesystem::FileEventType::FILE_CREATED, path, priority);
            }
        }
    }

    void ResourceManager::enumerateResources()
    {
        assert(!mEnumerated);

#if ENABLE_PLUGINS
        Plugins::PluginManager &pMgr = Plugins::PluginManager::getSingleton();
        for (auto &section : pMgr) {
            for (Plugins::Plugin &p : section) {
                if (!p.isLoaded(pMgr.selection()))
                    continue;
                const Plugins::BinaryInfo *info = p.info();
                if (info->mDataPath.empty())
                    continue;
                Platform::Filesystem::Path path = info->mDataPath;
                if (path.isRelative()) {
                    path = *sourceDirPath / path;
                }
                registerResourceLocation(path, p.name(), 75);
            }
        }

        if (!bakeOutputList->empty()) {
            registerResourceLocation(bakeOutputList->parentPath(), "Generated", 50);
        }

#else
        registerResourceLocation(Platform::Filesystem::shippingPath() / "data", "Game", 80);
#endif

        std::map<std::string, std::vector<ResourceLoaderBase *>, std::less<>> loaderByExtension = getLoaderByExtension();

        for (const std::pair<const Platform::Filesystem::Path, PathProperties> &p : mResourcePaths) {
            updateResources(Platform::Filesystem::FileEventType::FILE_CREATED, p.first, p.second.mPriority, loaderByExtension);
        }

        mEnumerated = true;
    }

    std::pair<std::string, Platform::Filesystem::Path> ResourceManager::makeRelative(const Platform::Filesystem::Path &path) const
    {
        for (const std::pair<const Platform::Filesystem::Path, PathProperties> &p : mResourcePaths) {
            Platform::Filesystem::Path relative = path.relative(p.first);
            if (!relative.empty()) {
                return { p.second.mIdentifier, std::move(relative) };
            }
        }

        if (!bakeOutputList->empty()) {
            Platform::Filesystem::Path relative = path.relative(bakeOutputList->parentPath());
            if (!relative.empty()) {
                return { "Generated", std::move(relative) };
            }
        }

        return std::make_pair("", "");
    }

    Platform::Filesystem::Path ResourceManager::getProjectPath(std::string_view name) const
    {
        auto it = std::ranges::find(mResourcePaths, name, [](const std::pair<const Platform::Filesystem::Path, PathProperties> &p) { return p.second.mIdentifier; });
        if (it == mResourcePaths.end())
            return {};
        return it->first;
    }

    Threading::Task<bool> ResourceManager::init()
    {
        enumerateResources();

        for (const std::unique_ptr<ResourceLoaderBase> &loader : mCollector) {
            co_await loader->callInit();
        }

        if (!Core::Root::getSingleton().toolMode())
            taskQueue()->queueTask(update());

        co_return true;
    }

    Threading::Task<void> ResourceManager::finalize()
    {
        mFileWatcher.clear();

        for (const std::unique_ptr<ResourceLoaderBase> &loader : mCollector) {
            co_await loader->callFinalize();
        }
    }

    Platform::Filesystem::Path ResourceManager::findResourceFile(std::string_view fileName)
    {
        for (const std::pair<const Platform::Filesystem::Path, PathProperties> &p : mResourcePaths) {
            for (Platform::Filesystem::Path p : Platform::Filesystem::listFilesRecursive(p.first)) {
                if (p.filename().str() == fileName)
                    return p;
            }
        }
        return {};
    }

    Threading::Task<void> ResourceManager::update()
    {
        while (taskQueue()->running()) {
            std::vector<Platform::Filesystem::FileEvent> events = mFileWatcher.fetchChangesReduced();

            std::map<std::string, std::vector<ResourceLoaderBase *>, std::less<>> loaderByExtension = getLoaderByExtension();

            for (const Platform::Filesystem::FileEvent &event : events) {
                updateResource(event.mType, event.mPath, mResourcePaths.at(event.mPath).mPriority, loaderByExtension);
            }

            co_await 1s;
        }
    }

    void ResourceManager::waitForInit()
    {
        mEnumerated.wait();
    }

    Threading::TaskQueue *ResourceManager::taskQueue()
    {
        return mRoot.taskQueue();
    }

    void ResourceManager::updateResources(Platform::Filesystem::FileEventType event, const Platform::Filesystem::Path &path, int priority)
    {
        updateResources(event, path, priority, getLoaderByExtension());
    }

    void ResourceManager::updateResources(Platform::Filesystem::FileEventType event, const Platform::Filesystem::Path &path, int priority, const std::map<std::string, std::vector<ResourceLoaderBase *>, std::less<>> &loaderByExtension)
    {
        LOG_DEBUG("Scanning for Resources in: " << path);

        for (Platform::Filesystem::Path p : Platform::Filesystem::listFilesRecursive(path)) {
            updateResource(event, p, priority, loaderByExtension);
        }
    }

    void ResourceManager::updateResource(Platform::Filesystem::FileEventType event, const Platform::Filesystem::Path &path, int priority, const std::map<std::string, std::vector<ResourceLoaderBase *>, std::less<>> &loaderByExtension)
    {
        LOG_DEBUG("Updating Resource: " << path << " (" << event << ")");

        std::string extension = StringUtil::toLower(path.extension());

        auto it = loaderByExtension.find(extension);
        if (it != loaderByExtension.end()) {
            for (ResourceLoaderBase *loader : it->second) {
                auto [resource, created] = loader->addResource(path);

                switch (event) {
                case Platform::Filesystem::FileEventType::FILE_CREATED:
                    if (!created && path != resource->path()) {
                        int otherPriority = mResourcePaths.at(resource->path()).mPriority;
                        if (priority > otherPriority || (priority == otherPriority && loader->extensionIndex(extension) < loader->extensionIndex(resource->path().extension()))) {
                            resource->setPath(path);
                            loader->updateResourceData(resource);
                        }
                    }
                    break;
                case Platform::Filesystem::FileEventType::FILE_MODIFIED:
                    if (!created)
                        loader->updateResourceData(resource);
                    break;
                }
            }
        }
    }

    std::map<std::string, std::vector<ResourceLoaderBase *>, std::less<>> ResourceManager::getLoaderByExtension()
    {
        std::map<std::string, std::vector<ResourceLoaderBase *>, std::less<>> loaderByExtension;

        for (const std::unique_ptr<ResourceLoaderBase> &loader : mCollector) {
            for (const std::string &ext : loader->fileExtensions()) {
                loaderByExtension[ext].push_back(loader.get());
            }
        }
        return loaderByExtension;
    }

    std::map<std::pair<std::string, Platform::Filesystem::Path>, std::vector<ResourceBase *>> ResourceManager::buildResourceList()
    {
        std::map<std::pair<std::string, Platform::Filesystem::Path>, std::vector<ResourceBase *>> result;
        for (const std::unique_ptr<ResourceLoaderBase> &loader : mCollector) {
            for (ResourceBase *res : loader->resources()) {
                Platform::Filesystem::Path path = res->path();
                if (!path.empty()) {
                    const auto &[project, relPath] = makeRelative(path);
                    if (project.empty()) {
                        LOG_WARNING("Resource " << path << " is not in a registered resource location, skipping writing it to the output list.");
                        continue;
                    }
                    result[{ project, relPath }].push_back(res);
                }
            }
        }
        return result;
    }

    bool ResourceManager::SubDirCompare::operator()(const Platform::Filesystem::Path &first, const Platform::Filesystem::Path &second) const
    {
        auto [firstEnd, secondEnd] = std::mismatch(first.str().begin(), first.str().end(), second.str().begin(), second.str().end());
        if (firstEnd == first.str().end() || secondEnd == second.str().end())
            return false;
        return first < second;
    }

}
}
