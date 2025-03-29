#pragma once

#include "Meta/keyvalue/scopeptr.h"

namespace Engine {

struct MADGINE_ROOT_EXPORT KeyValueRegistry {
    static void registerGlobal(const char *name, ScopePtr ptr);
    static void registerWorkGroupLocal(const char *name, ScopePtr ptr);
    static void unregisterGlobal(ScopePtr ptr);
    static void unregisterWorkGroupLocal(ScopePtr ptr);

    static const std::map<std::string_view, ScopePtr> &globals();
    static const std::map<std::string_view, ScopePtr> &workgroupLocals();
};

template <typename T>
struct KeyValueWorkGroupLocal : T {

    using decay_t = T;

    template <typename... Args>
    requires std::constructible_from<T, Args...>
    KeyValueWorkGroupLocal(const char *name, Args &&... args)
        : T(std::forward<Args>(args)...)
    {
        KeyValueRegistry::registerWorkGroupLocal(name, this);
    }

    ~KeyValueWorkGroupLocal()
    {
        KeyValueRegistry::unregisterWorkGroupLocal(this);
    }
};

template <typename T>
struct KeyValueGlobal : T {

    using decay_t = T;

    template <typename... Args>
    KeyValueGlobal(const char *name, Args &&... args)
        : T(std::forward<Args>(args)...)
    {
        KeyValueRegistry::registerGlobal(name, this);
    }

    ~KeyValueGlobal()
    {
        KeyValueRegistry::unregisterGlobal(this);
    }
};

}