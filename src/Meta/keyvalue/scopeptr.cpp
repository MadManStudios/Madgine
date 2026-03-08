#include "../metalib.h"

#include "scopeptr.h"

#include "metatable.h"
#include "scopefield.h"
#include "scopeiterator.h"
#include "virtualscope.h" //enforce export of VirtualScopeBase<void>

namespace Engine {

ScopeField ScopePtr::operator[](std::string_view key) const
{
    return *find(key);
}

/*bool ScopePtr::isEditable(const std::string &key) const
{
    return (*find(key)).isEditable();
}*/

ScopeIterator ScopePtr::find(std::string_view key) const
{
    return mType->find(key, *this);
}

ScopeIterator ScopePtr::begin() const
{
    return { *this, mType ? mType->mMembers : nullptr };
}

ScopeIterator ScopePtr::end() const
{
    return { *this, nullptr };
}

std::string ScopePtr::name() const
{
    if (mScope)
        return mType->name(*this);
    else
        return "<NULL>";
}

void ScopePtr::moveAssign(ScopePtr other) const
{
    mType->moveAssign(*this, other);
}

KeyValueResult ScopePtr::call(ValueType &retVal, const ArgumentList &args) const
{
    return mType->call(*this, retVal, args);
}

}