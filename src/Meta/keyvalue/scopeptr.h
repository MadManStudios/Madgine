#pragma once

#include "metatable.h"
#include "valuetype_desc.h"

namespace Engine {

struct META_EXPORT ScopePtr {

    constexpr ScopePtr() = default;

    template <typename T>
    ScopePtr(T *t)
        : ScopePtr(resolveCustomScopePtr(t))
    {
    }

    constexpr ScopePtr(const ScopePtr &other)
        : mScope(other.mScope)
        , mType(other.mType)
    {
    }

    ScopePtr(void *scope, const MetaTable *type)
        : mScope(scope)
        , mType(type)
    {
    }

    bool operator==(const ScopePtr &other) const
    {
        return mScope == other.mScope && mType == other.mType;
    }

    auto operator<=>(const ScopePtr &other) const = default;

    explicit operator bool() const
    {
        return mScope != nullptr;
    }

    ScopeIterator find(std::string_view key) const;
    ScopeField operator[](std::string_view key) const;
    // bool isEditable(const std::string &key) const;
    ScopeIterator begin() const;
    ScopeIterator end() const;

    std::string name() const;

    void moveAssign(ScopePtr other) const;

    void call(ValueType &retVal, const ArgumentList &args) const;

    void *mScope = nullptr;
    const MetaTable *mType = nullptr;
};

template <typename T>
    requires(!std::is_pointer_v<T>)
T *scope_cast(const ScopePtr &ptr)
{
    size_t offset = 0;
    if (!ptr.mType->isDerivedFrom<std::remove_const_t<T>>(&offset))
        std::terminate();

    return static_cast<T *>(reinterpret_cast<void *>(reinterpret_cast<std::byte *>(ptr.mScope) + offset));
}

}