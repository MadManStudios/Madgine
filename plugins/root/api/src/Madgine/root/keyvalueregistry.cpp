#include "../rootlib.h"

#include "keyvalueregistry.h"

#include "Generic/projections.h"

#include "Modules/threading/workgroupstorage.h"

namespace Engine {
namespace Core {

    std::map<std::string_view, Reflect::ScopePtr> sGlobalRegistry;
    Threading::WorkgroupLocal<std::map<std::string_view, Reflect::ScopePtr>> sWorkGroupLocalRegistry;

    void KeyValueRegistry::registerGlobal(const char *name, Reflect::ScopePtr ptr)
    {
        [[maybe_unused]] auto pib = sGlobalRegistry.try_emplace(name, ptr);
        assert(pib.second);
    }

    void KeyValueRegistry::registerWorkGroupLocal(const char *name, Reflect::ScopePtr ptr)
    {
        [[maybe_unused]] auto pib = sWorkGroupLocalRegistry->try_emplace(name, ptr);
        assert(pib.second);
    }

    void KeyValueRegistry::unregisterGlobal(Reflect::ScopePtr ptr)
    {
        auto it = std::ranges::find(sGlobalRegistry, ptr, projectionPairSecond);
        sGlobalRegistry.erase(it);
    }

    void KeyValueRegistry::unregisterWorkGroupLocal(Reflect::ScopePtr ptr)
    {
        auto it = std::ranges::find(*sWorkGroupLocalRegistry, ptr, projectionPairSecond);
        sWorkGroupLocalRegistry->erase(it);
    }

    const std::map<std::string_view, Reflect::ScopePtr> &KeyValueRegistry::globals()
    {
        return sGlobalRegistry;
    }

    const std::map<std::string_view, Reflect::ScopePtr> &KeyValueRegistry::workgroupLocals()
    {
        return sWorkGroupLocalRegistry;
    }

}
}