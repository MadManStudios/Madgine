#pragma once

#include "scopeptr.h"

namespace Engine {

struct META_EXPORT ScopeField {

    ScopeField(const ScopePtr &ptr, const Accessor *pointer);

    void value(ValueType &retVal) const;

    ScopeField &operator=(const ValueType &v);

    const char *key() const;

    bool isEditable() const;
    const ExtendedValueTypeDesc &type() const;

private:
    ScopePtr mScope;
    const Accessor *mPointer;
};

}