#pragma once

#include "../serialize/context.h"

namespace Engine {
namespace Type {

    struct META_EXPORT BaseStorage {
        void toValue(Reflect::Value &retVal);
        Reflect::Result fromValue(const Reflect::Value &val);

        const StorageOps &mType;
    };

    template <typename T>
    struct Storage : BaseStorage {
        template <typename... Args>
        Storage(const StorageOps &type, Args &&...args)
            : BaseStorage(type)
            , mObject(std::forward<Args>(args)...)
        {
        }

        T mObject;
    };

    struct META_EXPORT StorageDeleter {
        void operator()(BaseStorage *) const;
    };

    using AllocationPtr = std::unique_ptr<BaseStorage, StorageDeleter>;

    struct META_EXPORT InlineStorage : BaseStorage {

        InlineStorage(const StorageOps &ops, const Reflect::ArgumentList &args);
        ~InlineStorage();

        void toValue(Reflect::Value &retVal);
        Reflect::Result fromValue(const Reflect::Value &val);

        uintptr_t mDummy[2] = { 0, 0 };

    private:
        friend struct Serialize::Operations<InlineStorage>;
        
        META_EXPORT friend Serialize::StreamResult tag_invoke(const Serialize::apply_map_t &, InlineStorage &storage, Serialize::FormattedSerializeStream &in, bool success, Serialize::ContextPtr context);

        template <typename... Configs, typename Context>
        friend void tag_invoke(Serialize::set_active_t<Configs...>, InlineStorage &storage, bool active, bool existenceChanged, Context &&)
        {
        }
    };

    struct META_EXPORT AllocationStorage : BaseStorage {

        AllocationStorage(const StorageOps &ops, const Reflect::ArgumentList &args);

        void toValue(Reflect::Value &retVal);
        Reflect::Result fromValue(const Reflect::Value &val);

        Serialize::StreamResult read(Serialize::FormattedSerializeStream &in, const char *name, Serialize::ContextPtr context);
        void write(Serialize::FormattedSerializeStream &out, const char *name, Serialize::ContextPtr context) const;
        Serialize::StreamResult applyMap(Serialize::FormattedSerializeStream &in, bool success, Serialize::ContextPtr context);

        AllocationPtr mAllocation;
    };

}

namespace Serialize {

    template <>
    struct META_EXPORT Operations<Type::InlineStorage> {
        static StreamResult read(Serialize::FormattedSerializeStream &in, Type::InlineStorage &storage, const char *name, ContextPtr context);
        static void write(Serialize::FormattedSerializeStream &out, const Type::InlineStorage &storage, const char *name, ContextPtr context);

        static StreamResult visitStream(FormattedSerializeStream &in, const char *name, const StreamVisitor &visitor, size_t depth);
    };

}

}