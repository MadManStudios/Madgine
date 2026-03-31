#include "../metalib.h"

#include "scopefield.h"

#include "accessor.h"

#include "valuetype.h"

namespace Engine {

ScopeField::ScopeField(const ScopePtr &ptr, const Accessor *pointer)
    : mScope(ptr)
    , mPointer(pointer)
{
    assert(ptr);
}

KeyValueResult ScopeField::value(ValueType &retVal) const
{
    return mPointer->mGetter(mPointer, retVal, ValueType { mScope });
}

KeyValueResult ScopeField::operator=(const ValueType &v)
{
    return mPointer->mSetter(mPointer, ValueType { mScope }, v);
}

const char *ScopeField::key() const
{
    return mPointer->mName;
}

bool ScopeField::isEditable() const
{
    return mPointer->mSetter;
}

const ExtendedValueTypeDesc &ScopeField::type() const
{
    return mPointer->mType;
}

}