#include "../metalib.h"

#include "scopeiterator.h"

#include "accessor.h"

#include "scopefield.h"

#include "metatable.h"

namespace Engine {

ScopeIterator::ScopeIterator(ScopePtr scope, const std::pair<const char *, Accessor> *pointer)
    : mScope(scope)
    , mCurrentTable(scope.mType)
    , mPointer(scope ? pointer : nullptr)
{
    if (mPointer) {
        checkDerived();
    }
}

bool ScopeIterator::operator==(const ScopeIterator &other) const
{
    assert(mScope == other.mScope);
    if (mPointer == other.mPointer)
        return true;
    if (!mPointer)
        return !other.mPointer->first;
    else
        return !mPointer->first;
}

bool ScopeIterator::operator!=(const ScopeIterator &other) const
{
    assert(mScope.mScope == other.mScope.mScope);
    if (mPointer == other.mPointer)
        return false;
    if (!mPointer)
        return other.mPointer->first;
    else
        return mPointer->first;
}

ScopeField ScopeIterator::operator*() const
{
    return { mScope, mPointer };
}

Proxy<ScopeField> ScopeIterator::operator->() const
{
    return { mScope, mPointer };
}

void ScopeIterator::operator++()
{
    assert(mPointer);
    ++mPointer;
    checkDerived();
}

void ScopeIterator::checkDerived()
{
    while (!mPointer->first && mCurrentTable->mBase) {
        mCurrentTable = *mCurrentTable->mBase;
        mPointer = mCurrentTable->mMembers;
    }
}

ScopeIterator ScopeIterator::end() const
{
    return { mScope, nullptr };
}

}