#pragma once

#include "Generic/proxy.h"

#include "scopefield.h"

namespace Engine {
namespace Reflect {

    struct META_EXPORT ScopeIterator {

        using iterator_category = std::forward_iterator_tag;
        using value_type = ScopeField;
        using difference_type = ptrdiff_t;
        using pointer = void;
        using reference = ScopeField;

        ScopeIterator() = default;
        ScopeIterator(const Value &scope, const Accessor *pointer);

        bool operator==(const ScopeIterator &other) const;

        bool operator!=(const ScopeIterator &other) const;

        ScopeField operator*() const;
        Proxy<ScopeField> operator->() const;

        ScopeIterator &operator++();
        ScopeIterator operator++(int);

        ScopeIterator end() const;

    private:
        void check();

        Value mScope;
        const MetaTable *mCurrentTable = nullptr;
        const Accessor *mPointer = nullptr;
    };

}
}