#include "../rootlib.h"

#include "keyvalueregistry.h"

#include "Modules/threading/workgroupstorage.h"

#include "Generic/projections.h"

namespace Engine {

std::map<std::string_view, ScopePtr> sGlobalRegistry;
Threading::WorkgroupLocal<std::map<std::string_view, ScopePtr>> sWorkGroupLocalRegistry;

void KeyValueRegistry::registerGlobal(const char *name, ScopePtr ptr)
{
    auto pib = sGlobalRegistry.try_emplace(name, ptr);
    assert(pib.second);
}

void KeyValueRegistry::registerWorkGroupLocal(const char *name, ScopePtr ptr)
{
    auto pib = sWorkGroupLocalRegistry->try_emplace(name, ptr);
    assert(pib.second);
}

void KeyValueRegistry::unregisterGlobal(ScopePtr ptr)
{
    auto it = std::ranges::find(sGlobalRegistry, ptr, projectionPairSecond);
    sGlobalRegistry.erase(it);
}

void KeyValueRegistry::unregisterWorkGroupLocal(ScopePtr ptr)
{
    auto it = std::ranges::find(*sWorkGroupLocalRegistry, ptr, projectionPairSecond);
    sWorkGroupLocalRegistry->erase(it);
}

const std::map<std::string_view, ScopePtr> &KeyValueRegistry::globals()
{
    return sGlobalRegistry;
}

const std::map<std::string_view, ScopePtr> &KeyValueRegistry::workgroupLocals()
{
    return sWorkGroupLocalRegistry;
}

}