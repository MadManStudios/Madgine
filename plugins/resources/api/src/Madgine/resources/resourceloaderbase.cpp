#include "../resourceslib.h"

#include "resourceloaderbase.h"

#include "resourcemanager.h"

#include "Meta/keyvalue/metatable_impl.h"

METATABLE_BEGIN(Engine::Resources::ResourceLoaderBase)
METATABLE_END(Engine::Resources::ResourceLoaderBase)

namespace Engine {
namespace Resources {

    ResourceLoaderBase::ResourceLoaderBase(std::vector<std::string> &&extensions, const ResourceLoaderSettings &settings)
        : mExtensions(std::forward<std::vector<std::string>>(extensions))
        , mSettings(settings)
    {
    }

    Threading::Task<bool> ResourceLoaderBase::init()
    {
        co_return true;
    }

    Threading::Task<void> ResourceLoaderBase::finalize()
    {
        co_return;
    }

    Threading::Task<BakeResult> ResourceLoaderBase::bakeResources(std::vector<Filesystem::Path> &resources, const Filesystem::Path &intermediateDir)
    {
        co_return BakeResult::NOTHING_TO_DO;
    }

    const std::vector<std::string> &ResourceLoaderBase::fileExtensions() const
    {
        return mExtensions;
    }

    size_t ResourceLoaderBase::extensionIndex(std::string_view ext) const
    {
        return std::ranges::find(mExtensions, ext) - mExtensions.begin();
    }

    std::vector<const MetaTable *> ResourceLoaderBase::resourceTypes() const
    {
        return {};
    }

    Threading::TaskQueue *ResourceLoaderBase::loadingTaskQueue() const
    {
        return ResourceManager::getSingleton().taskQueue();
    }

}
}
