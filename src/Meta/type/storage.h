#pragma once

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

        META_EXPORT friend Serialize::StreamResult tag_invoke(const Serialize::apply_map_t &, InlineStorage &storage, Serialize::CallerHierarchyFormattedSerializeStream in, bool success);

        template <typename... Configs>
        friend void tag_invoke(Serialize::set_active_t<Configs...>, InlineStorage &storage, bool active, bool existenceChanged, const CallerHierarchyBasePtr &)
        {
        }
    };

    struct META_EXPORT AllocationStorage : BaseStorage {

        AllocationStorage(const StorageOps &ops, const Reflect::ArgumentList &args);

        void toValue(Reflect::Value &retVal);
        Reflect::Result fromValue(const Reflect::Value &val);

        Serialize::StreamResult read(Serialize::CallerHierarchyFormattedSerializeStream in, const char *name);
        void write(Serialize::CallerHierarchyFormattedSerializeStream out, const char *name) const;
        Serialize::StreamResult applyMap(Serialize::CallerHierarchyFormattedSerializeStream in, bool success);

        AllocationPtr mAllocation;
    };

}

namespace Serialize {

    template <>
    struct META_EXPORT Operations<Type::InlineStorage> {
        static StreamResult read(Serialize::CallerHierarchyFormattedSerializeStream in, Type::InlineStorage &storage, const char *name);
        static void write(Serialize::CallerHierarchyFormattedSerializeStream out, const Type::InlineStorage &storage, const char *name);

        static StreamResult visitStream(CallerHierarchyFormattedSerializeStream in, const char *name, const StreamVisitor &visitor, size_t depth);
    };

}

}