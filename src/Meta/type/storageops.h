#pragma once

#include "../reflect/result.h"
#include "../reflect/util.h"
#include "../serialize/operations.h"
#include "storage.h"
#include "storageops_forward.h"
#include "typenames.h"

namespace Engine {
namespace Type {

    struct Constructor {
        bool (*mMatcher)(const StorageOps &, const Reflect::ArgumentList &) = nullptr;
        Reflect::Result (*mInvoker)(const StorageOps &, BaseStorage &, const Reflect::ArgumentList &, size_t) = nullptr;
    };

    struct META_EXPORT StorageOps {

        typedef void(MoveAssign)(BaseStorage &, BaseStorage &&);
        typedef void(ToValue)(Reflect::Value &, BaseStorage &, size_t);
        typedef Reflect::Result(FromValue)(BaseStorage &, const Reflect::Value &, size_t);
        typedef void(Destructor)(BaseStorage *, size_t);

        typedef Serialize::StreamResult(Read)(const StorageOps &, Serialize::CallerHierarchyFormattedSerializeStream, BaseStorage &, const char *, size_t);
        typedef void(Write)(Serialize::CallerHierarchyFormattedSerializeStream, const BaseStorage &, const char *, size_t);
        typedef Serialize::StreamResult(ApplyMap)(BaseStorage &, Serialize::CallerHierarchyFormattedSerializeStream, bool, size_t);

        template <typename T>
        constexpr MoveAssign *moveAssign()
        {
            if constexpr (std::is_assignable_v<T, T &&>) {
                return [](BaseStorage &target, BaseStorage &&other) {
                    static_cast<Storage<T> &>(target).mObject = std::move(static_cast<Storage<T> &>(other).mObject);
                };
            } else {
                return nullptr;
            }
        }

        constexpr StorageOps(const StorageOps **self, const char *name, const Constructor *constructors, MoveAssign *moveAssign, ToValue *toValue, FromValue *fromValue, Reflect::ExtendedType type, Destructor *destructor, Read *read, Write *write, ApplyMap *applyMap)
            : mSelf(self)
            , mTypeName(name)
            , mConstructors(constructors)
            , mMoveAssign(moveAssign)
            , mToValue(toValue)
            , mFromValue(fromValue)
            , mType(type)
            , mDestructor(destructor)
            , mRead(read)
            , mWrite(write)
            , mApplyMap(applyMap)
        {
        }

        template <typename T>
        constexpr StorageOps(const char *name, type_holder_t<T>, const Constructor *constructors)
            : mSelf(&storageOps<T>)
            , mTypeName(name)
            , mConstructors(constructors)
            , mMoveAssign(moveAssign<T>())
            , mToValue([](Reflect::Value &retVal, BaseStorage &value, size_t inlineSize) {
                if (inlineSize >= sizeof(T)) {
                    Reflect::toValue(retVal, std::ref(static_cast<Storage<T> &>(value).mObject));
                } else {
                    return static_cast<AllocationStorage &>(value).mAllocation->toValue(retVal);
                }
            })
            , mFromValue([](BaseStorage &target, const Reflect::Value &value, size_t inlineSize) -> Reflect::Result {
                if constexpr (std::is_copy_assignable_v<T>) {
                    if (inlineSize >= sizeof(T)) {
                        return Reflect::call([&](const T &val) -> Reflect::Result {
                            static_cast<Storage<T> &>(target).mObject = val;
                            return {};
                        },
                            value);
                    } else {
                        return static_cast<AllocationStorage &>(target).mAllocation->fromValue(value);
                    }
                } else {
                    return REFLECT_UNKNOWN_ERROR() << "Type '" << typeid(T).name() << "' is not copy assignable";
                }
            })
            , mType(Reflect::toType<T>())
            , mDestructor([](BaseStorage *storage, size_t inlineSize) {
                assert(inlineSize > 0);
                if (inlineSize >= sizeof(T)) {
                    static_cast<Storage<T> *>(storage)->mObject.~T();
                } else {
                    delete &static_cast<AllocationStorage *>(storage)->mAllocation;
                }
            })
            , mRead([](const StorageOps &ops, Serialize::CallerHierarchyFormattedSerializeStream in, BaseStorage &storage, const char *name, size_t inlineSize) {
                if (inlineSize >= sizeof(T)) {
                    return Serialize::read(in, static_cast<Storage<T> &>(storage).mObject, name);
                } else {
                    return static_cast<AllocationStorage &>(storage).read(in, name);
                }
            })
            , mWrite([](Serialize::CallerHierarchyFormattedSerializeStream out, const BaseStorage &storage, const char *name, size_t inlineSize) {
                if (inlineSize >= sizeof(T)) {
                    Serialize::write(out, static_cast<const Storage<T> &>(storage).mObject, name);
                } else {
                    static_cast<const AllocationStorage &>(storage).write(out, name);
                }
            })
            , mApplyMap([](BaseStorage &storage, Serialize::CallerHierarchyFormattedSerializeStream in, bool success, size_t inlineSize) {
                if (inlineSize >= sizeof(T)) {
                    return Serialize::apply_map(static_cast<Storage<T> &>(storage).mObject, in, success);
                } else {
                    return static_cast<AllocationStorage &>(storage).applyMap(in, success);
                }
            })
        {
        }

        Reflect::Result construct(BaseStorage &storage, const Reflect::ArgumentList &args, size_t inlineSize) const;
        Serialize::StreamResult read(Serialize::CallerHierarchyFormattedSerializeStream, BaseStorage &, const char *, size_t) const;

        const StorageOps **mSelf;
        const char *mTypeName;
        const Constructor *mConstructors;
        MoveAssign *mMoveAssign;
        ToValue *mToValue;
        FromValue *mFromValue;
        Reflect::ExtendedType mType;
        Destructor *mDestructor;

        Read *mRead;
        Write *mWrite;
        ApplyMap *mApplyMap;
    };

    namespace __Type_impl__ {

        template <typename T>
        struct StorageOpsRegistrator {
            StorageOpsRegistrator()
            {
                registerStorageOps(*storageOps<T>);
            }
            ~StorageOpsRegistrator()
            {
                unregisterStorageOps(*storageOps<T>);
            }
        };

    }
}
}