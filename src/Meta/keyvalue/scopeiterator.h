#pragma once

#include "Generic/proxy.h"
#include "scopefield.h"

namespace Engine {

struct META_EXPORT ScopeIterator {

    ScopeIterator(ScopePtr scope, const Accessor *pointer);

    bool operator==(const ScopeIterator &other) const;

    bool operator!=(const ScopeIterator &other) const;

    ScopeField operator*() const;
    Proxy<ScopeField> operator->() const;

    void operator++();

    ScopeIterator end() const;

private:
    void check();

    ScopePtr mScope;
    const MetaTable *mCurrentTable;
    const Accessor *mPointer;
};

}