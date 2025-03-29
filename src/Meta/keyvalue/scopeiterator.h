#pragma once

#include "Generic/proxy.h"
#include "scopefield.h"

namespace Engine {

struct META_EXPORT ScopeIterator {

    ScopeIterator(ScopePtr scope, const std::pair<const char *, Accessor> *pointer);

    bool operator==(const ScopeIterator &other) const;

    bool operator!=(const ScopeIterator &other) const;

    ScopeField operator*() const;
    Proxy<ScopeField> operator->() const;

    void operator++();

    ScopeIterator end() const;

private:
    void checkDerived();

    ScopePtr mScope;
    const MetaTable *mCurrentTable;
    const std::pair<const char *, Accessor> *mPointer;
};

}