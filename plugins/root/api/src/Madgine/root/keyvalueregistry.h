#pragma once

#include "Meta/reflect/scopeptr.h"

namespace Engine {
namespace Core {

    struct MADGINE_ROOT_EXPORT KeyValueRegistry {
        static void registerGlobal(const char *name, Reflect::ScopePtr ptr);
        static void registerWorkGroupLocal(const char *name, Reflect::ScopePtr ptr);
        static void unregisterGlobal(Reflect::ScopePtr ptr);
        static void unregisterWorkGroupLocal(Reflect::ScopePtr ptr);

        static const std::map<std::string_view, Reflect::ScopePtr> &globals();
        static const std::map<std::string_view, Reflect::ScopePtr> &workgroupLocals();
    };

    template <typename T>
    struct KeyValueWorkGroupLocal : T {

        using meta_t = T;

        template <typename... Args>
            requires std::constructible_from<T, Args...>
        KeyValueWorkGroupLocal(const char *name, Args &&...args)
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

        using meta_t = T;

        template <typename... Args>
        KeyValueGlobal(const char *name, Args &&...args)
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
}