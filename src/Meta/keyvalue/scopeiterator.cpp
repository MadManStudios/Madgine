#include "../metalib.h"

#include "scopeiterator.h"

#include "accessor.h"
#include "metatable.h"
#include "scopefield.h"

namespace Engine {

ScopeIterator::ScopeIterator(const ValueType &scope, const Accessor *pointer)
    : mScope(scope)
    , mCurrentTable(*scope.type().mSecondary.mMetaTable)
    , mPointer(!scope.is<std::monostate>() ? pointer : nullptr)
{
    if (mPointer) {
        check();
    }
}

bool ScopeIterator::operator==(const ScopeIterator &other) const
{
    assert(mScope == other.mScope);
    if (mPointer == other.mPointer)
        return true;
    if (!mPointer)
        return !other.mPointer->mName;
    else
        return !mPointer->mName;
}

bool ScopeIterator::operator!=(const ScopeIterator &other) const
{
    assert(mScope == other.mScope);
    if (mPointer == other.mPointer)
        return false;
    if (!mPointer)
        return other.mPointer->mName;
    else
        return mPointer->mName;
}

ScopeField ScopeIterator::operator*() const
{
    return { mScope, mPointer };
}

Proxy<ScopeField> ScopeIterator::operator->() const
{
    return { mScope, mPointer };
}

ScopeIterator &ScopeIterator::operator++()
{
    assert(mPointer);
    ++mPointer;
    check();
    return *this;
}

ScopeIterator ScopeIterator::operator++(int)
{
    assert(mPointer);
    ScopeIterator it = *this;
    ++mPointer;
    check();
    return it;
}

void ScopeIterator::check()
{
    bool check = true;
    while (check) {
        check = false;
        if (!mPointer->mName) {
            if (mCurrentTable->mBase) {
                mCurrentTable = *mCurrentTable->mBase;
                mPointer = mCurrentTable->mMembers;
                check = true;
            }
        } else {
            if (mPointer->mCheck && !mPointer->mCheck(mPointer, mScope)) {
                ++mPointer;
                check = true;
            }
        }
    }
}

ScopeIterator ScopeIterator::end() const
{
    return { mScope, nullptr };
}

}