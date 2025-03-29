#include "../metalib.h"

#include "scopefield.h"

#include "accessor.h"

namespace Engine {

ScopeField::ScopeField(const ScopePtr &ptr, const std::pair<const char *, Accessor> *pointer)
    : mScope(ptr)
    , mPointer(pointer)
{
    assert(ptr);
}

void ScopeField::value(ValueType &retVal) const
{
    mPointer->second.mGetter(retVal, mScope);
}

ScopeField &ScopeField::operator=(const ValueType &v)
{
    mPointer->second.mSetter(mScope, v);
    return *this;
}

const char *ScopeField::key() const
{
    return mPointer->first;
}

bool ScopeField::isEditable() const
{
    return mPointer->second.mSetter;
}

const ExtendedValueTypeDesc &ScopeField::type() const
{
    return mPointer->second.mType;
}

}