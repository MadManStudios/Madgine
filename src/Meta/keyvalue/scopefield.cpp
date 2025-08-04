#include "../metalib.h"

#include "scopefield.h"

#include "accessor.h"

namespace Engine {

ScopeField::ScopeField(const ScopePtr &ptr, const Accessor *pointer)
    : mScope(ptr)
    , mPointer(pointer)
{
    assert(ptr);
}

void ScopeField::value(ValueType &retVal) const
{
    mPointer->mGetter(mPointer, retVal, mScope);
}

ScopeField &ScopeField::operator=(const ValueType &v)
{
    mPointer->mSetter(mPointer, mScope, v);
    return *this;
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