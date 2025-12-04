#include "../metalib.h"

#include "ownedscopeptr.h"

#include "metatable.h"
#include "proxyscopebase.h"

namespace Engine {

OwnedScopePtr::OwnedScopePtr(std::shared_ptr<ProxyScopeBase> ptr)
    : mScope(std::move(ptr))
{
}

std::string OwnedScopePtr::name() const
{
    return get().name();
}

bool OwnedScopePtr::operator==(const OwnedScopePtr &other) const
{
    return mScope == other.mScope;
}

ScopePtr OwnedScopePtr::get() const
{
    return mScope->proxyScopePtr();
}

void OwnedScopePtr::construct(const MetaTable *type)
{
    *this = type->mConstructors[0]();
}

const MetaTable *OwnedScopePtr::type() const
{
    return get().mType;
}

}