#pragma once

#include "Generic/closure.h"
#include "Generic/container/emplace.h"

#include "Interfaces/filesystem/filewatcher.h"

#include "Meta/keyvalue/ownedscopeptr.h"

#include "Modules/threading/globalstorage.h"
#include "Modules/uniquecomponent/uniquecomponent.h"
#include "Modules/uniquecomponent/uniquecomponentcollector.h"

#include "handle.h"
#include "ptr.h"
#include "resource.h"
#include "resourcebase.h"
#include "resourceloaderbase.h"
#include "resourceloadercollector.h"

namespace Engine {
namespace Resources {

    MADGINE_RESOURCES_EXPORT ResourceLoaderBase &getLoaderByIndex(size_t i);
    MADGINE_RESOURCES_EXPORT void waitForIOThread();

    MADGINE_RESOURCES_EXPORT Threading::TaskFuture<bool> queueLoad(Threading::Task<bool> task, Threading::TaskQueue *queue);
    MADGINE_RESOURCES_EXPORT Threading::TaskFuture<void> queueUnload(Threading::Task<void> task, Threading::TaskQueue *queue);

    template <typename T, typename _Data, typename _Base = ResourceLoaderCollector::Base>
    struct InstanceLoaderInterface : _Base {
        using Base = _Base;
        using Data = _Data;

        struct Resource : ResourceLoaderBase::Resource {

            using ResourceLoaderBase::Resource::Resource;

            Threading::TaskFuture<bool> loadData(Data &data)
            {
                return T::load(static_cast<typename T::Resource *>(this), data);
            }

            Threading::Task<bool> loadTask(Data &data)
            {
                return T::loadTask(static_cast<typename T::Resource *>(this), data);
            }

            /* Threading::TaskFuture<void> forceUnload()
            {
                return T::unload(this);
            }*/

            /* Data *dataPtr()
            {
                return T::getDataPtr(loadData());
            }*/
        };

        using Ctor = Closure<Threading::Task<bool>(T *, Data &, Resource *)>;
        using UnnamedCtor = Closure<Threading::Task<bool>(T *, Data &, Resource *)>;

        template <typename Loader = T, typename C = void>
        static typename Loader::Ctor toCtor(C &&ctor)
        {
            static_assert(!std::is_same_v<C, typename Loader::Ctor>);
            return [ctor { std::forward<C>(ctor) }](T *loader, Data &data, Resource *res) {
                return Threading::make_task(ctor, static_cast<Loader *>(loader), static_cast<typename Loader::Data &>(data), static_cast<typename Loader::Resource *>(res));
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

    template <typename T, typename _Data, typename _Base = InstanceLoaderInterface<T, _Data>>
    struct InstanceLoaderImpl : _Base {

        using Interface = _Base;
        using Base = _Base;
        using Data = _Data;

        using Resource = Resource<T>;

        using Ctor = Closure<Threading::Task<bool>(T *, Data &, Resource *)>;

        using Base::Base;

        static T &getSingleton()
        {
            return static_cast<T &>(getLoaderByIndex(UniqueComponent::component_index<T>()));
        }

        static Threading::TaskFuture<bool> load(std::string_view name, Data &data, T *loader = &getSingleton())
        {
            if (name.empty())
                return {};
            Resource *res = get(name, loader);
            if (!res)
                return {};
            return load(res, data, loader);
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

        template <typename C = Ctor>
        static Resource *getOrCreateManual(std::string_view name, const Filesystem::Path &path = {}, C &&ctor = {}, T *loader = &getSingleton())
        {
            auto pib = loader->mResources.try_emplace(
                std::string { name }, std::string { name }, path, Interface::template toCtor<T>(std::forward<C>(ctor)));

            Resource *resource = &pib.first->second;

            return resource;
        }

        static Threading::Task<bool> loadTask(Resource *resource, Data &data, T *loader = &getSingleton())
        {
            return resource->mCtor(loader, data, resource);
        }

        static Threading::TaskFuture<bool> load(Resource *resource, Data &data, T *loader = &getSingleton())
        {
            return queueLoad(loadTask(resource, data, loader), loader->loadingTaskQueue());
        }

        template <typename C = Ctor>
        static Threading::TaskFuture<bool> loadManual(std::string_view name, const Filesystem::Path &path = {}, C &&ctor = {}, T *loader = &getSingleton())
        {
            return load(getOrCreateManual(
                            name, path, std::forward<C>(ctor),
                            loader),
                loader);
        }

        std::pair<ResourceBase *, bool> addResource(const Filesystem::Path &path, std::string_view name = {}) override
        {
            std::string actualName { name.empty() ? path.stem() : name };
            auto pib = mResources.try_emplace(actualName, actualName, path);

            return std::make_pair(&pib.first->second, pib.second);
        }

        void updateResourceData(ResourceBase *resource) override
        {
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
    };

    template <typename T, typename _Data>
    struct InstanceLoader : ResourceLoaderComponent<T, VirtualScope<T, InstanceLoaderImpl<T, _Data, InstanceLoaderInterface<T, _Data>>>> {

        using ResourceLoaderComponent<T, VirtualScope<T, InstanceLoaderImpl<T, _Data, InstanceLoaderInterface<T, _Data>>>>::ResourceLoaderComponent;
    };

}
}

#define INSTANCELOADER(Loader)                                                    \
    UNIQUECOMPONENT(Loader)                                                       \
                                                                                  \
    METATABLE_BEGIN_EX(1, Loader)                                                 \
        MEMBER_EX(2, mResources)                                                  \
    METATABLE_END_EX(4, Loader)                                                   \
                                                                                  \
    METATABLE_BEGIN_BASE_EX(5, Loader::Resource, Engine::Resources::ResourceBase) \
    METATABLE_END_EX(6, Loader::Resource)
