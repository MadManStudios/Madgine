#pragma once

#include "Generic/offsetptr.h"

#include "table_forward.h"

#include "keyvalueresult.h"

namespace Engine {

template <typename T>
T *scope_cast(const ScopePtr &ptr);

using Constructor = OwnedScopePtr (*)();

struct META_EXPORT MetaTable {

    typedef void(MoveAssign)(const ScopePtr &, const ScopePtr &);

    template <typename T>
    constexpr MoveAssign *moveAssign()
    {
        if constexpr (std::is_assignable_v<T, T &&>) {
            return [](const ScopePtr &target, const ScopePtr &other) {
                *scope_cast<T>(target) = std::move(*scope_cast<T>(other));
            };
        } else {
            return nullptr;
        }
    }

    constexpr MetaTable(const MetaTable **self, const char *name, const Accessor *members, const Constructor *constructors = nullptr, MoveAssign *moveAssign = nullptr)
        : mSelf(self)
        , mTypeName(name)
        , mBase(nullptr)
        , mBaseOffset(nullptr)
        , mMembers(members)
        , mConstructors(constructors)
        , mMoveAssign(moveAssign)
    {
    }

    template <typename T, typename Base>
    constexpr MetaTable(const char *name, type_holder_t<T>, type_holder_t<Base>, const Accessor *members, const Constructor *constructors)
        : mSelf(&table<T>)
        , mTypeName(name)
        , mBase(&table<Base>)
        , mBaseOffset([]() { return OffsetPtr { type_holder<T>, type_holder<Base> }; })
        , mMembers(members)
        , mConstructors(constructors)
        , mMoveAssign(moveAssign<T>())
    {
    }

    template <typename T>
    constexpr MetaTable(const char *name, type_holder_t<T>, type_holder_t<void>, const Accessor *members, const Constructor *constructors)
        : mSelf(&table<T>)
        , mTypeName(name)
        , mBase(nullptr)
        , mBaseOffset(nullptr)
        , mMembers(members)
        , mConstructors(constructors)
        , mMoveAssign(moveAssign<T>())
    {
    }

    ScopeIterator find(std::string_view key, ScopePtr scope) const;

    KeyValueResult call(ScopePtr scope, ValueType &retVal, const ArgumentList &args) const;

    void moveAssign(ScopePtr scope, ScopePtr other) const;

    template <typename T>
    bool isDerivedFrom(OffsetPtr *offset = nullptr) const
    {
        return isDerivedFrom(table<T>, offset);
    }
    bool isDerivedFrom(const MetaTable *baseType, OffsetPtr *offset = nullptr) const;

    std::string name(ScopePtr scope) const;

    const MetaTable **mSelf;
    const char *mTypeName;
    const MetaTable **mBase;
    OffsetPtr (*mBaseOffset)();
    const Accessor *mMembers;
    const Constructor *mConstructors;
    MoveAssign *mMoveAssign;

    mutable const MetaTable *mNext = nullptr;
    mutable const MetaTable **mPrev = nullptr;
};

META_EXPORT const MetaTable *&sTypeList();

META_EXPORT void registerType(const MetaTable &t);
META_EXPORT void unregisterType(const MetaTable &t);

template <typename T>
struct MetaTableRegistrator {
    MetaTableRegistrator()
    {
        registerType(*table<T>);
    }
    ~MetaTableRegistrator()
    {
        unregisterType(*table<T>);
    }
};

}
