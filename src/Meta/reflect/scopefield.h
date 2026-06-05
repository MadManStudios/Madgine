#pragma once

#include "scopeptr.h"
#include "value.h"

namespace Engine {
namespace Reflect {

    struct META_EXPORT ScopeField {

        ScopeField(const Value &ptr, const Accessor *pointer);

        Result value(Value &retVal) const;

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