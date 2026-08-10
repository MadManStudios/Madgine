#pragma once

#include "scopeptr.h"
#include "value.h"

namespace Engine {
namespace Reflect {

    struct META_EXPORT ScopeField {

        ScopeField(const Value &ptr, const Accessor *pointer);

        Result value(Value &retVal, ContextPtr context = {}) const;
        Result set(const Value &val, ContextPtr context = {});
        Result operator=(const Value &v);

        const char *key() const;

        bool isEditable() const;
        const ExtendedType &type() const;
        AccessorFlags flags() const;

    private:
        Value mScope;
        const Accessor *mPointer;
    };

}
}