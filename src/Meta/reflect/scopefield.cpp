#include "../metalib.h"

#include "scopefield.h"

#include "accessor.h"

#include "value.h"

namespace Engine {
namespace Reflect {

    ScopeField::ScopeField(const Value &ptr, const Accessor *pointer)
        : mScope(ptr)
        , mPointer(pointer)
    {
        assert(!ptr.is<std::monostate>());
    }

    Result ScopeField::value(Value &retVal, ContextPtr context) const
    {
        return mPointer->mGetter(mPointer, retVal, Value { mScope }, context);
    }

    Result ScopeField::set(const Value &val, ContextPtr context)
    {
        return mPointer->mSetter(mPointer, Value { mScope }, val, context);
    }

    Result ScopeField::operator=(const Value &v)
    {
        return set(v);
    }

    const char *ScopeField::key() const
    {
        return mPointer->mName;
    }

    bool ScopeField::isEditable() const
    {
        return mPointer->mSetter;
    }

    const ExtendedType &ScopeField::type() const
    {
        return mPointer->mType;
    }

    AccessorFlags ScopeField::flags() const
    {
        return mPointer->mFlags;
    }

}
}