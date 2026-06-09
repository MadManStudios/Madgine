#pragma once

#include "type.h"
#include "result.h"

namespace Engine {
namespace Reflect {

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

        Result call(Value &retVal, const ArgumentList &args) const;

        void *mScope = nullptr;
        const MetaTable *mType = nullptr;
    };

}
}