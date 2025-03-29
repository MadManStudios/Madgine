#include "../metalib.h"

#include "accessor.h"
#include "metatable.h"
#include "scopefield.h"
#include "scopeiterator.h"
#include "valuetype.h"

namespace Engine {

ScopeIterator MetaTable::find(std::string_view key, ScopePtr scope) const
{
    for (const std::pair<const char *, Accessor> *p = mMembers; p->first; ++p) {
        if (key == p->first) {
            return { scope, p };
        }
    }
    if (mBase) {
        return (*mBase)->find(key, scope);
    } else {
        return { scope, nullptr };
    }
}

void MetaTable::call(ScopePtr scope, ValueType &retVal, const ArgumentList &args) const
{
    ScopeIterator op = find("__call", scope);
    if (op == scope.end())
        throw "No call-operator for type!";
    ValueType f;
    op->value(f);
    f.as<BoundApiFunction>()(retVal, args);
}

void MetaTable::moveAssign(ScopePtr scope, ScopePtr other) const
{
    mMoveAssign(scope, other);
}

bool MetaTable::isDerivedFrom(const MetaTable *baseType, size_t *offset) const
{
    if (this == baseType)
        return true;
    if (offset && mBaseOffset)
        *offset += mBaseOffset();
    return mBase && (*mBase)->isDerivedFrom(baseType, offset);
}

std::string MetaTable::name(ScopePtr scope) const
{
    ScopeIterator nameIt = find("Name", scope);
    if (nameIt != scope.end()) {
        ValueType name;
        nameIt->value(name);
        if (name.is<std::string>()) {
            return name.as<std::string>();
        }
    }
    ScopeIterator proxyIt = find("__proxy", scope);
    if (proxyIt != scope.end()){
        ValueType proxy;
        proxyIt->value(proxy);
        if (proxy.is<ScopePtr>()) {
            return proxy.as<ScopePtr>().name();
        }
    }
    return "<"s + mTypeName + ">";
}

    const MetaTable *&sTypeList()
    {
        static const MetaTable *sDummy = nullptr;
        return sDummy;
    }

    void registerType(const MetaTable &f)
    {
        if (sTypeList()) {
            sTypeList()->mPrev = &f.mNext;
        }
        f.mNext = std::exchange(sTypeList(), &f);
    }

    void unregisterType(const MetaTable &f)
    {
    }


}