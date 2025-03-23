#pragma once

#include "resourcebase.h"
#include "resourceloadercollector.h"

#include "Modules/threading/globalstorage.h"

#include "handle.h"
#include "ptr.h"
#include "resource.h"

#include "resourceloaderbase.h"

#include "Modules/uniquecomponent/uniquecomponent.h"
#include "Modules/uniquecomponent/uniquecomponentcollector.h"

#include "Interfaces/filesystem/filewatcher.h"

#include "Generic/closure.h"

#include "Generic/container/emplace.h"

#include "Meta/keyvalue/ownedscopeptr.h"

namespace Engine {
namespace Resources {

    MADGINE_RESOURCES_EXPORT ResourceLoaderBase &getLoaderByIndex(size_t i);
    MADGINE_RESOURCES_EXPORT void waitForIOThread();

    MADGINE_RESOURCES_EXPORT Threading::TaskFuture<bool> queueLoad(Threading::Task<bool> task, Threading::TaskQueue *queue);
    MADGINE_RESOURCES_EXPORT Threading::TaskFuture<void> queueUnload(Threading::Task<void> task, Threading::TaskQueue *queue);

    template <typename T, typename _Data, typename _Container = std::list<Placeholder<0>>, typename _Storage = Threading::GlobalStorage, typename _Base = ResourceLoaderCollector::Base>
    struct ResourceLoaderInterface : _Base {
        using Base = _Base;
        using Data = _Data;
        using Container = _Container;
        using Storage = _Storage;

        using ResourceDataInfo = ResourceDataInfo<T>;

        using DataContainer = typename replace<Container>::template type<ResourceData<T>>;

        using traits = container_traits<DataContainer>;

        using Handle = Handle<T, typename traits::handle>;
        using Ptr = Ptr<T, Data>;
        using OriginalHandle = Handle;
        using OriginalPtr = Ptr;

        using Ctor = Closure<Threading::Task<bool>(T *, Data &, ResourceDataInfo &, Filesystem::FileEventType event)>;
        using UnnamedCtor = Closure<Threading::Task<bool>(T *, Data &)>;

        struct Resource : ResourceLoaderBase::Resource {

            using ResourceLoaderBase::Resource::Resource;

            Handle loadData()
            {
                return T::load(static_cast<typename T::Resource *>(this));
            }

            /* Threading::TaskFuture<void> forceUnload()
            {
                return T::unload(this);
            }*/

            /* Data *dataPtr()
            {
                return T::getDataPtr(loadData());
            }*/

            typename Storage::template container_type<typename traits::handle> mData;
        };

        template <typename Loader = T, typename C = void>
        static typename Loader::Ctor toCtor(C &&ctor)
        {
            static_assert(!std::is_same_v<C, typename Loader::Ctor>);
            return [ctor { std::forward<C>(ctor) }](T *loader, Data &data, ResourceDataInfo &info, Filesystem::FileEventType event) {
                return Threading::make_task(LIFT(TupleUnpacker::invoke), ctor, static_cast<Loader *>(loader), static_cast<typename Loader::Data &>(data), info, event);
            };
        }

        template <typename Loader = T>
        static typename Loader::Ctor toCtor(typename Loader::Ctor &&ctor)
        {
            return std::move(ctor);
        }

        using Base::Base;

        static T &getSingleton()
        {
            return static_cast<T &>(getLoaderByIndex(UniqueComponent::component_index<T>()));
        }
    };

    template <typename T, typename _Data, typename _Base = ResourceLoaderInterface<T, _Data>>
    struct ResourceLoaderImpl : _Base {

        using Interface = _Base;
        using Base = _Base;
        using Data = _Data;

        using ResourceDataInfo = typename Interface::ResourceDataInfo;
        using Resource = Resource<T>;

        using DataContainer = typename replace<typename Base::Container>::template type<ResourceData<T>>;

        using traits = container_traits<DataContainer>;

        static_assert(!traits::remove_invalidates_handles);

        using Handle = Handle<T, typename traits::handle>;
        using Ptr = Ptr<T, Data>;

        using Ctor = Closure<Threading::Task<bool>(T *, Data &, ResourceDataInfo &, Filesystem::FileEventType event)>;

        using Base::Base;

        static T &getSingleton()
        {
            return static_cast<T &>(getLoaderByIndex(UniqueComponent::component_index<T>()));
        }

        static Handle load(std::string_view name, T *loader = &getSingleton())
        {
            if (name.empty())
                return {};
            Resource *res = get(name, loader);
            if (!res)
                return {};
            return load(res, Filesystem::FileEventType::FILE_CREATED, loader);
        }

        static Resource *get(std::string_view name, T *loader = &getSingleton())
        {
            waitForIOThread();

            auto it = loader->mResources.find(name);
            if (it != loader->mResources.end())
                return &it->second;
            else
                return nullptr;
        }

        static ResourceDataInfo *getInfo(const Handle &handle, T *loader = nullptr)
        {
            ResourceData<T> *data = getData(handle, loader);
            if (!data)
                return nullptr;
            else
                return &data->mInfo;
        }

        static ResourceData<T> *getData(const Handle &handle, T *loader = nullptr)
        {
            if (!handle)
                return nullptr;
            if constexpr (traits::has_dependent_handle) {
                if (!loader)
                    loader = &getSingleton();
                return &(*loader->mData)[handle.mData];
            } else {
                return handle.mData;
            }
        }

        template <typename C = Ctor>
        static Resource *getOrCreateManual(std::string_view name, const Filesystem::Path &path = {}, C &&ctor = {}, T *loader = &getSingleton())
        {
            auto pib = loader->mResources.try_emplace(
                std::string { name }, std::string { name }, path, Interface::template toCtor<T>(std::forward<C>(ctor)));

            Resource *resource = &pib.first->second;
            if (pib.second) {
                loader->resourceAdded(resource);
            }

            return resource;
        }

        static Handle create(Resource *resource, Filesystem::FileEventType event = Filesystem::FileEventType::FILE_CREATED, T *loader = nullptr)
        {
            Handle handle { (typename traits::handle) * resource->mData };
            assert(event == Filesystem::FileEventType::FILE_CREATED || handle);
            if (!handle || event != Filesystem::FileEventType::FILE_CREATED) {
                if (event == Filesystem::FileEventType::FILE_CREATED || !loader->mSettings.mInplaceReload) {
                    if (!loader)
                        loader = &getSingleton();
                    auto it = emplace(*loader->mData, loader->mData->end(), resource);
                    it->mHolder = traits::toPositionHandle(*loader->mData, it);
                    handle = traits::toHandle(*loader->mData, it->mHolder);
                    *resource->mData = (decltype(*resource->mData))handle.mData;
                }
            }
            return handle;
        }

        static Handle load(Resource *resource, Filesystem::FileEventType event = Filesystem::FileEventType::FILE_CREATED, T *loader = nullptr)
        {
            Handle handle { (typename traits::handle) * resource->mData };

            if (!handle || event != Filesystem::FileEventType::FILE_CREATED) {
                if (!loader)
                    loader = &getSingleton();
                handle = create(resource, event, loader);

                ResourceDataInfo &info = *getInfo(handle, loader);
                info.setLoadingTask(resource->mCtor(loader, *getDataPtr(handle, loader, false), info, event), loader->loadingTaskQueue());
            }

            return handle;
        }

        static Threading::TaskFuture<void> unload(Resource *resource, T *loader = &getSingleton())
        {
            Handle handle { (typename traits::handle) * resource->mData };
            return unload(handle, loader);
        }

        static Threading::TaskFuture<void> unload(const Handle &handle, T *loader = &getSingleton())
        {
            if (!handle)
                return Threading::TaskFuture<void>::make_ready();

            ResourceDataInfo &info = *getInfo(handle, loader);
            Threading::TaskFuture<void> task = info.unloadingTask();

            if (!task.valid()) {
                task = info.setUnloadingTask(Threading::make_task(&T::unloadImpl, std::move(loader), *getDataPtr(handle, loader, false)), loader->loadingTaskQueue());
            }

            return task;
        }

        static void unload(std::unique_ptr<Data> ptr, T *loader = &getSingleton())
        {
            if (!ptr)
                return;

            queueUnload(Threading::make_task(&T::unloadImpl, std::move(loader), *ptr).then([ptr { std::move(ptr) }]() mutable { ptr.reset(); }), loader->loadingTaskQueue());
        }

        static void resetHandle(const Handle &handle, T *loader = nullptr)
        {
            assert(handle);
            if (!handle.info()->decRef()) {
                if (!loader)
                    loader = &getSingleton();

                Threading::TaskFuture<void> task = unload(handle, loader);

                Resource *resource = handle.resource();
                queueUnload(task.then([&data { *loader->mData }, handle { getData(handle)->mHolder }]() {
                    typename traits::iterator it = traits::toIterator(data, handle);
                    data.erase(it);
                }),
                    loader->loadingTaskQueue());

                if ((typename traits::handle) * resource->mData == handle.mData)
                    *resource->mData = {};
            }
        }

        static Handle refreshHandle(const Handle &handle, T *loader = nullptr)
        {
            if (!handle)
                return {};

            Resource *resource = handle.resource();
            if ((typename traits::handle) * resource->mData != handle.mData) {
                return load(resource, Filesystem::FileEventType::FILE_CREATED, loader);
            }
            return {};
        }

        template <typename C = Ctor>
        static Handle loadManual(std::string_view name, const Filesystem::Path &path = {}, C &&ctor = {}, T *loader = &getSingleton())
        {
            return load(getOrCreateManual(
                            name, path, std::forward<C>(ctor),
                            loader),
                Filesystem::FileEventType::FILE_CREATED, loader);
        }

        static Ptr createUnnamed()
        {
            return std::make_unique<Data>();
        }

        template <typename C>
        static Threading::Task<bool> loadUnnamedTask(Ptr &ptr, C &&ctor, T *loader)
        {
            ptr = createUnnamed();
            return Threading::make_task(std::forward<C>(ctor), std::move(loader), *ptr);
        }

        template <typename C>
        static Threading::TaskFuture<bool> loadUnnamed(Ptr &ptr, C &&ctor, T *loader = &getSingleton())
        {
            return queueLoad(loadUnnamedTask(ptr, std::forward<C>(ctor), loader), loader->loadingTaskQueue());
        }

        static Data *getDataPtr(const Handle &handle, T *loader = nullptr, bool verified = true)
        {
            ResourceData<T> *data = getData(handle, loader);
            if (!data)
                return nullptr;
            else
                return data->verified(verified);
        }

        std::pair<ResourceBase *, bool> addResource(const Filesystem::Path &path, std::string_view name = {}) override
        {
            std::string actualName { name.empty() ? path.stem() : name };
            auto pib = mResources.try_emplace(actualName, actualName, path);

            if (pib.second)
                this->resourceAdded(&pib.first->second);

            return std::make_pair(&pib.first->second, pib.second);
        }

        void updateResourceData(ResourceBase *resource) override
        {
            if (static_cast<T *>(this)->mSettings.mAutoReload) {
                Resource *res = static_cast<Resource *>(resource);
                if (*res->mData) {
                    if (static_cast<T *>(this)->mSettings.mInplaceReload) {
                        load(res, Filesystem::FileEventType::FILE_MODIFIED);
                    } else {
                        *res->mData = {};
                    }
                }
            }
        }

        typename std::map<std::string, Resource>::iterator begin()
        {
            return mResources.begin();
        }

        typename std::map<std::string, Resource>::iterator end()
        {
            return mResources.end();
        }

        virtual std::vector<ResourceBase *> resources() override
        {
            std::vector<ResourceBase *> result;
            std::ranges::transform(mResources, std::back_inserter(result), [](std::pair<const std::string, Resource> &p) {
                return &p.second;
            });
            return result;
        }

        virtual std::vector<std::pair<std::string_view, ScopePtr>> typedResources() override
        {
            std::vector<std::pair<std::string_view, ScopePtr>> result;
            std::ranges::transform(mResources, std::back_inserter(result), [](std::pair<const std::string, Resource> &p) {
                return std::make_pair(std::string_view { p.first }, &p.second);
            });
            return result;
        }

        virtual std::vector<const MetaTable *> resourceTypes() const override
        {
            std::vector<const MetaTable *> result = Base::resourceTypes();
            result.push_back(table<decayed_t<Resource>>);
            return result;
        }

        std::map<std::string, Resource, std::less<>> mResources;

        typename Base::Storage::template container_type<DataContainer> mData;
    };

    template <typename T, typename _Data, typename Container = std::list<Placeholder<0>>, typename Storage = Threading::GlobalStorage>
    struct ResourceLoader : ResourceLoaderComponent<T, VirtualScope<T, ResourceLoaderImpl<T, _Data, ResourceLoaderInterface<T, _Data, Container, Storage>>>> {

        using ResourceLoaderComponent<T, VirtualScope<T, ResourceLoaderImpl<T, _Data, ResourceLoaderInterface<T, _Data, Container, Storage>>>>::ResourceLoaderComponent;
    };

}
}

#define RESOURCELOADER(Loader)                                                    \
    UNIQUECOMPONENT(Loader)                                                       \
                                                                                  \
    METATABLE_BEGIN_EX(1, Loader)                                                 \
    MEMBER_EX(2, mResources)                                                      \
    METATABLE_END_EX(4, Loader)                                                   \
                                                                                  \
    METATABLE_BEGIN_BASE_EX(5, Loader::Resource, Engine::Resources::ResourceBase) \
    METATABLE_END_EX(6, Loader::Resource)                                         \
                                                                                  \
    SERIALIZETABLE_BEGIN_EX(9, Loader::Handle)                                    \
    ENCAPSULATED_FIELD_EX(10, Name, name, loadSerialize)                          \
    SERIALIZETABLE_END_EX(11, Loader::Handle)
