#pragma once

#include "scopeptr.h"

#include "valuetype.h"

namespace Engine {

struct META_EXPORT ScopeField {

    ScopeField(const ValueType &ptr, const Accessor *pointer);

    KeyValueResult value(ValueType &retVal) const;

    KeyValueResult operator=(const ValueType &v);

    const char *key() const;

    bool isEditable() const;
    const ExtendedValueTypeDesc &type() const;
    AccessorFlags flags() const;

private:
    ValueType mScope;
    const Accessor *mPointer;
};

}