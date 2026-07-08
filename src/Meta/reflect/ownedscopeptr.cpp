#include "../metalib.h"

#include "ownedscopeptr.h"

#include "metatable.h"
#include "proxyscopebase.h"

namespace Engine {
namespace Reflect {

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

    Result OwnedScopePtr::construct(const MetaTable *type, const ArgumentList &args)
    {
        std::unique_ptr<ProxyScopeBase> newScope;
        Result result = type->mConstructors[0].mInvoker(newScope, args);
        if (!result) {
            mScope = std::move(newScope);
        }
        return result;
    }

    const MetaTable *OwnedScopePtr::type() const
    {
        return get().mType;
    }

}
}