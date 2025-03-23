#pragma once

#include "resourceloader.h"

#include "Meta/keyvalue/virtualscope.h"

namespace Engine {
namespace Resources {

    template <typename T, typename _Data, typename _Container = std::list<Placeholder<0>>, typename _Storage = Threading::GlobalStorage>
    struct VirtualResourceLoaderBase : ResourceLoaderVirtualBase<T, ResourceLoaderInterface<T, _Data, _Container, _Storage>> {

        using Base = ResourceLoaderInterface<T, _Data, _Container, _Storage>;

        using ResourceLoaderVirtualBase<T, ResourceLoaderInterface<T, _Data, _Container, _Storage>>::ResourceLoaderVirtualBase;

        static typename Base::Handle load(std::string_view name, T *loader = &Base::getSingleton())
        {
            return loader->loadVImpl(name);
        }

        static typename Base::Handle load(typename Base::Resource *resource, T *loader = &Base::getSingleton())
        {
            return loader->loadVImpl(resource);
        }

        static Threading::TaskFuture<void> unload(typename Base::Resource *resource, T *loader = &Base::getSingleton())
        {
            return loader->unloadVImpl(resource);
        }

        static void unload(std::unique_ptr<_Data> ptr, T *loader = &Base::getSingleton())
        {
            return loader->unloadVImpl(std::move(ptr));
        }

        static void resetHandle(const typename Base::Handle &handle, T *loader = &Base::getSingleton())
        {
            return loader->resetHandleVImpl(handle);
        }

        static typename Base::Handle refreshHandle(const typename Base::Handle &handle, T *loader = &Base::getSingleton())
        {
            return loader->refreshHandleVImpl(handle);
        }

        template <typename C = typename Base::Ctor>
        static typename Base::Handle loadManual(std::string_view name, const Filesystem::Path &path = {}, C &&ctor = {}, T *loader = &Base::getSingleton())
        {
            return loader->loadManualVImpl(
                name, path, Base::toCtor(std::forward<C>(ctor)));
        }

        template <typename C>
        static Threading::TaskFuture<bool> loadUnnamed(typename Base::Ptr &ptr, C &&ctor, T *loader = &Base::getSingleton())
        {
            return queueLoad(loadUnnamedTask(ptr, std::forward<C>(ctor), loader), loader->loadingTaskQueue());
        }

        template <typename C = typename Base::Ctor>
        static Threading::Task<bool> loadUnnamedTask(typename Base::Ptr &ptr, C &&ctor, T *loader = &Base::getSingleton())
        {
            ptr = loader->createUnnamedVImpl();
            return Threading::make_task(std::forward<C>(ctor), (T *)loader, *ptr);
        }

        static typename Base::Data *getDataPtr(const typename Base::Handle &handle, T *loader = nullptr, bool verified = true)
        {
            if (!handle)
                return nullptr;
            if constexpr (container_traits<typename Base::DataContainer>::has_dependent_handle) {
                if (!loader)
                    loader = &Base::getSingleton();
                return loader->getDataPtrVImpl(handle, verified);
            } else {
                return handle.mData->verified(verified);
            }
        }

        static typename Base::ResourceDataInfo *getInfo(const typename Base::Handle &handle, T *loader = nullptr)
        {
            if (!handle)
                return nullptr;
            if constexpr (container_traits<typename Base::DataContainer>::has_dependent_handle) {
                if (!loader)
                    loader = &Base::getSingleton();
                return loader->getInfoVImpl(handle);
            } else {
                return &handle.mData->mInfo;
            }
        }

        virtual std::vector<const MetaTable *> resourceTypes() const override
        {
            std::vector<const MetaTable *> result = ResourceLoaderBase::resourceTypes();
            result.push_back(table<typename Base::Resource>);
            return result;
        }

        virtual typename Base::Handle loadManualVImpl(std::string_view name, const Filesystem::Path &path = {}, typename Base::Ctor ctor = {}) = 0;
        virtual typename Base::Ptr createUnnamedVImpl() = 0;
        virtual typename Base::Handle loadVImpl(std::string_view name) = 0;
        virtual typename Base::Handle loadVImpl(typename Base::Resource *resource) = 0;
        virtual Threading::TaskFuture<void> unloadVImpl(typename Base::Resource *resource) = 0;
        virtual void unloadVImpl(std::unique_ptr<_Data> ptr) = 0;
        virtual void resetHandleVImpl(const typename Base::Handle &handle) = 0;
        virtual typename Base::Handle refreshHandleVImpl(const typename Base::Handle &handle) = 0;
        virtual typename Base::Data *getDataPtrVImpl(const typename Base::Handle &handle, bool verified) = 0;
        virtual typename Base::ResourceDataInfo *getInfoVImpl(const typename Base::Handle &handle) = 0;
    };

    template <typename T, typename _Data, typename _Base>
    struct VirtualResourceLoaderImpl : UniqueComponent::VirtualComponentImpl<T, VirtualScope<T, ResourceLoaderImpl<T, _Data, _Base>>, _Base> {

        using Data = _Data;
        using Base = _Base;

        using Self = UniqueComponent::VirtualComponentImpl<T, VirtualScope<T, ResourceLoaderImpl<T, Data, Base>>, _Base>;

        using Self::Self;

        virtual typename Base::OriginalHandle loadManualVImpl(std::string_view name, const Filesystem::Path &path = {}, typename Base::Ctor ctor = {}) override
        {
            return Self::loadManual(name, path, std::move(ctor), static_cast<T *>(this));
        }
        virtual typename Base::OriginalPtr createUnnamedVImpl() override
        {
            return Self::createUnnamed();
        }
        virtual typename Base::OriginalHandle loadVImpl(std::string_view name) override
        {
            return Self::load(name, static_cast<T *>(this));
        }
        virtual typename Base::OriginalHandle loadVImpl(typename Base::Resource *resource) override
        {
            return Self::load(static_cast<typename Self::Resource *>(resource), Filesystem::FileEventType::FILE_CREATED, static_cast<T *>(this));
        }
        virtual Threading::TaskFuture<void> unloadVImpl(typename Base::Resource *resource) override
        {
            return Self::unload(static_cast<typename Self::Resource *>(resource), static_cast<T *>(this));
        }
        virtual void unloadVImpl(std::unique_ptr<typename Base::Data> data) override
        {
            return Self::unload(std::unique_ptr<Data> { static_cast<Data *>(data.release()) });
        }
        virtual void resetHandleVImpl(const typename Base::OriginalHandle &handle) override
        {
            Self::resetHandle(handle, static_cast<T *>(this));
        }
        virtual typename Base::OriginalHandle refreshHandleVImpl(const typename Base::OriginalHandle &handle) override
        {
            return Self::refreshHandle(handle, static_cast<T *>(this));
        }
        virtual typename Base::Data *getDataPtrVImpl(const typename Base::OriginalHandle &handle, bool verified) override
        {
            return Self::getDataPtr(handle, static_cast<T *>(this), verified);
        }
        virtual typename Base::ResourceDataInfo *getInfoVImpl(const typename Base::OriginalHandle &handle) override
        {
            return Self::getInfo(handle, static_cast<T *>(this));
        }
    };

}
}

#define VIRTUALRESOURCELOADERBASE(Loader)                                         \
    VIRTUALUNIQUECOMPONENTBASE(Loader)                                            \
                                                                                  \
    METATABLE_BEGIN_EX(1, Loader)                                                 \
    METATABLE_END_EX(2, Loader)                                                   \
                                                                                  \
    METATABLE_BEGIN_BASE_EX(3, Loader::Resource, Engine::Resources::ResourceBase) \
    METATABLE_END_EX(4, Loader::Resource)                                         \
                                                                                  \
    SERIALIZETABLE_BEGIN_EX(8, Loader::Handle)                                    \
    ENCAPSULATED_FIELD_EX(9, Name, name, loadSerialize)                           \
    SERIALIZETABLE_END_EX(10, Loader::Handle)

#define VIRTUALRESOURCELOADERIMPL(Loader, Base)                  \
    UNIQUECOMPONENT(Loader)                                      \
                                                                 \
    METATABLE_BEGIN_BASE_EX(1, Loader, Base)                     \
    MEMBER_EX(2, mResources)                                     \
    METATABLE_END_EX(3, Loader)                                  \
                                                                 \
    METATABLE_BEGIN_BASE_EX(4, Loader::Resource, Base::Resource) \
    METATABLE_END_EX(5, Loader::Resource)                        \
                                                                 \
    SERIALIZETABLE_BEGIN_EX(9, Loader::Handle)                   \
    ENCAPSULATED_FIELD_EX(10, Name, name, loadSerialize)         \
    SERIALIZETABLE_END_EX(11, Loader::Handle)