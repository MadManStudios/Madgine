#pragma once

#include "table_forward.h"

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

    constexpr MetaTable(const MetaTable **self, const char *name, const std::pair<const char *, Accessor> *members, const Constructor *constructors = nullptr, MoveAssign *moveAssign = nullptr)
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
    constexpr MetaTable(const char *name, type_holder_t<T>, type_holder_t<Base>, const std::pair<const char *, Accessor> *members, const Constructor *constructors)
        : mSelf(&table<T>)
        , mTypeName(name)
        , mBase(&table<Base>)
        , mBaseOffset(&Engine::inheritance_offset<Base, T>)
        , mMembers(members)
        , mConstructors(constructors)
        , mMoveAssign(moveAssign<T>())
    {
    }

    template <typename T>
    constexpr MetaTable(const char *name, type_holder_t<T>, type_holder_t<void>, const std::pair<const char *, Accessor> *members, const Constructor *constructors)
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

    void call(ScopePtr scope, ValueType &retVal, const ArgumentList &args) const;

    void moveAssign(ScopePtr scope, ScopePtr other) const;

    template <typename T>
    bool isDerivedFrom(size_t *offset = nullptr) const
    {
        return isDerivedFrom(table<T>, offset);
    }
    bool isDerivedFrom(const MetaTable *baseType, size_t *offset = nullptr) const;

    std::string name(ScopePtr scope) const;

    const MetaTable **mSelf;
    const char *mTypeName;
    const MetaTable **mBase;
    size_t (*mBaseOffset)();
    const std::pair<const char *, Accessor> *mMembers;
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
