#include "../metalib.h"

#include "metatable.h"

#include "accessor.h"
#include "scopefield.h"
#include "scopeiterator.h"
#include "value.h"

namespace Engine {
namespace Reflect {

    ScopeIterator MetaTable::find(std::string_view key, const Value &scope) const
    {
        for (const Accessor *p = mMembers; p->mName; ++p) {
            if (key == p->mName) {
                return { scope, p };
            }
        }
        if (mBase) {
            return (*mBase)->find(key, scope);
        } else {
            return { scope, nullptr };
        }
    }

    Result MetaTable::call(const Value &scope, Value &retVal, const ArgumentList &args) const
    {
        ScopeIterator op = find("__call", scope);
        if (op == scope.end())
            throw "No call-operator for type!";
        Value f;
        REFLECT_PROPAGATE_ERROR(op->value(f));
        return f.as<BoundApiFunction>()(retVal, args);
    }

    void MetaTable::moveAssign(ScopePtr scope, ScopePtr other) const
    {
        mMoveAssign(scope, other);
    }

    bool MetaTable::isDerivedFrom(const MetaTable *baseType, OffsetPtr *offset) const
    {
        if (this == baseType)
            return true;
        if (offset && mBaseOffset)
            *offset += mBaseOffset();
        return mBase && (*mBase)->isDerivedFrom(baseType, offset);
    }

    std::string MetaTable::name(const Value &scope) const
    {
        ScopeIterator nameIt = find("Name", scope);
        if (nameIt != scope.end()) {
            Value name;
            nameIt->value(name);
            if (name.is<std::string>()) {
                return name.as<CoWString>();
            }
        }
        ScopeIterator proxyIt = find("__proxy", scope);
        if (proxyIt != scope.end()) {
            Value proxy;
            proxyIt->value(proxy);
            if (proxy.is<ScopePtr>()) {
                return proxy.as<ScopePtr>().name();
            }
        }
        return "<"s + mTypeName + ">";
    }

}
}