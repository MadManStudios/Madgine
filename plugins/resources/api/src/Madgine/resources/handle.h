#pragma once

#include "Meta/serialize/streams/streamresult.h"

#include "Modules/threading/task.h"
#include "Modules/threading/taskfuture.h"

namespace Engine {
namespace Resources {

    template <typename Loader, typename Data>
    struct Handle {

        using meta_t = typename Loader::Resource *;

        Handle() = default;

        Handle(const Handle &other)
            : mData(other.mData)
        {
            if (mData)
                info()->incRef();
        }

        Handle(Handle &&other)
            : mData(std::exchange(other.mData, {}))
        {
        }

        Handle(std::nullptr_t)
            : Handle()
        {
        }

        Handle(typename Loader::Resource *res)
        {
            *this = Loader::load(res);
        }

        Handle(std::string_view name)
        {
            *this = Loader::load(name);
        }

        template <size_t N>
        Handle(const char (&name)[N])
            : Handle(std::string_view { name })
        {
        }

        Handle(Data data)
            : mData(data)
        {
            if (mData)
                info()->incRef();
        }

        ~Handle()
        {
            reset();
        }

        template <typename Loader2, typename Data2>
        Handle(const Handle<Loader2, Data2> &other)
        {
            if constexpr (std::derived_from<Loader2, Loader> || std::derived_from<Loader, Loader2>) {
                mData = (Data)other.mData;
                if (mData)
                    info()->incRef();
            } else {
                static_assert(dependent_bool<Loader2, false>::value, "Invalid conversion-type for Handle!");
            }
        }

        Handle &operator=(const Handle &other)
        {
            reset();
            mData = other.mData;
            if (mData)
                info()->incRef();
            return *this;
        }

        Handle &operator=(Handle &&other)
        {
            std::swap(mData, other.mData);
            return *this;
        }

        Handle &operator=(typename Loader::Resource *res)
        {
            return *this = Loader::load(res);
        }

        auto operator<=>(const Handle<Loader, Data> &) const = default;

        const auto &operator*() const
        {
            return *Loader::getDataPtr(*this);
        }

        const auto operator->() const
        {
            return Loader::getDataPtr(*this);
        }

        operator const typename Loader::Data *() const
        {
            return Loader::getDataPtr(*this);
        }

        bool available() const
        {
            return *this && info()->verify();
        }

        const typename Loader::Data *getUnsafe() const
        {
            return Loader::getDataPtr(*this, nullptr, false);
        }

        typename Loader::ResourceDataInfo *info() const
        {
            return Loader::getInfo(*this);
        }

        typename Loader::Resource *resource() const
        {
            typename Loader::ResourceDataInfo *i = info();
            if (!i)
                return nullptr;
            return static_cast<typename Loader::Resource *>(i->resource());
        }

        std::string_view name() const
        {
            typename Loader::Resource *res = resource();
            return res ? res->name() : "";
        }

        explicit operator bool() const
        {
            return mData != Data {};
        }

        Threading::TaskFuture<bool> load(std::string_view name, Loader *loader = &Loader::getSingleton())
        {
            *this = Loader::load(name, loader);
            typename Loader::ResourceDataInfo *i = info();
            if (!i)
                return false;
            return i->loadingTask();
        }

        Threading::TaskFuture<bool> load(typename Loader::Resource *resource, Loader *loader = &Loader::getSingleton())
        {
            *this = Loader::load(resource, loader);
            typename Loader::ResourceDataInfo *i = info();
            if (!i)
                return false;
            return i->loadingTask();
        }

        Threading::TaskFuture<bool> loadSerialize(std::string_view name)
        {
            return load(name);
        }

        template <typename C = typename Loader::Ctor>
        Threading::TaskFuture<bool> create(std::string_view name, const Platform::Filesystem::Path &path = {}, C &&c = {}, Loader *loader = &Loader::getSingleton())
        {
            *this = Loader::loadManual(name, path, std::forward<C>(c), loader);
            typename Loader::ResourceDataInfo *i = info();
            if (!i)
                return false;
            return i->loadingTask();
        }

        template <typename C = typename Loader::Ctor>
        Threading::Task<bool> createTask(std::string_view name, const Platform::Filesystem::Path &path = {}, C &&c = {}, Loader *loader = &Loader::getSingleton())
        {
            *this = Loader::loadManual(name, path, std::forward<C>(c), loader);
            typename Loader::ResourceDataInfo *i = info();
            if (!i)
                return Threading::make_ready_task(false);
            return i->loadingTask();
        }

        void reset()
        {
            if (mData) {
                Loader::resetHandle(*this);
                mData = Data {};
            }
        }

        Handle refresh() const
        {
            return Loader::refreshHandle(*this);
        }

        typename Loader::Resource *customScopePtr() const
        {
            return resource();
        }

        template <typename Context>
        friend Serialize::StreamResult tag_invoke(const Serialize::apply_map_t &, Handle &h, Serialize::FormattedSerializeStream &in, bool success, Context &&context)
        {
            return {};
        }

        template <typename... Configs, typename Context>
        friend void tag_invoke(Serialize::set_active_t<Configs...>, Handle &h, bool active, bool existenceChanged, Context &&)
        {
        }

        Data mData = {};
    };

}

namespace Serialize {
    template <typename Loader, typename Data>
    struct Operations<Resources::Handle<Loader, Data>> {
        template <typename Context>
        static StreamResult read(Serialize::FormattedSerializeStream &in, Resources::Handle<Loader, Data> &handle, const char *name, Context &&context)
        {
            std::string resourceName;
            STREAM_PROPAGATE_ERROR(Serialize::read(in, resourceName, name));
            handle.load(resourceName);
            return {};
        }
        template <typename Context>
        static void write(Serialize::FormattedSerializeStream &out, const Resources::Handle<Loader, Data> &handle, const char *name, Context &&context)
        {
            Serialize::write(out, handle.name(), name);
        }

        static StreamResult visitStream(FormattedSerializeStream &in, const char *name, const StreamVisitor &visitor, size_t depth)
        {
            throw 0;
        }
    };
}

}