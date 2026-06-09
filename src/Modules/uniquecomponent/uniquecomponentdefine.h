#pragma once

#include "Generic/replace.h"

#include "annotations.h"

// base is included in __VA_ARGS__ to circumvent the problem with empty __VA_ARGS__ and ,
#define DECLARE_UNIQUE_COMPONENT2(ns, prefix, registry, component, /*base, */...)                                                                                                \
    namespace ns {                                                                                                                                                               \
    constexpr auto prefix##Header() { return __FILE__; }                                                                                                                         \
    using prefix##Registry = registry<STRINGIFY(ns::prefix##BaseRegistry), STRINGIFY(ns::prefix##Registry), prefix##Header, __VA_ARGS__>;                                        \
    using prefix##BaseRegistry = Engine::Plugins::Registry<STRINGIFY(ns::prefix##BaseRegistry), STRINGIFY(ns::prefix##Registry), prefix##Header, __VA_ARGS__>;                   \
    using prefix##Collector = Engine::Plugins::Collector<prefix##Registry>;                                                                                                      \
    template <typename C>                                                                                                                                                        \
    using prefix##Container = Engine::Plugins::Container<typename Engine::replace<C>::template type<std::unique_ptr<FIRST(__VA_ARGS__)>>, prefix##Registry, FIRST(__VA_ARGS__)>; \
    using prefix##Selector = Engine::Plugins::Selector<prefix##Registry, FIRST(__VA_ARGS__)>;                                                                                    \
    template <typename T, typename Base = FIRST(__VA_ARGS__)>                                                                                                                    \
    using prefix##Component = component<Engine::Plugins::Component<T, prefix##Collector, Base>>;                                                                                 \
    template <typename T, typename Base = FIRST(__VA_ARGS__)>                                                                                                                    \
    using prefix##VirtualBase = component<Engine::Plugins::VirtualComponentBase<T, prefix##Collector, Base>>;                                                                    \
    template <typename T, typename Base>                                                                                                                                         \
    using prefix##VirtualImpl = Engine::Plugins::VirtualComponentImpl<T, Base>;                                                                                                  \
    }

#define DECLARE_UNIQUE_COMPONENT(ns, prefix, /*base, */...) DECLARE_UNIQUE_COMPONENT2(ns, prefix, Engine::Plugins::Registry, std::type_identity_t, __VA_ARGS__)

#define DECLARE_NAMED_UNIQUE_COMPONENT(ns, prefix, /*base, */...) \
    DECLARE_UNIQUE_COMPONENT2(ns, prefix, Engine::Plugins::NamedRegistry, Engine::Plugins::NamedComponent, __VA_ARGS__)

#if defined(STATIC_BUILD)
#    define EXPORT_REGISTRY(Registry, BaseRegistry)
#else
#    define EXPORT_REGISTRY(Registry, BaseRegistry) DLL_EXPORT_VARIABLE3_ORDER(, BaseRegistry, Registry, Engine::Plugins::, registry, , {}, BaseRegistry)
#endif

#define DEFINE_UNIQUE_COMPONENT(ns, prefix) \
    EXPORT_REGISTRY(ns::prefix##Registry, ns::prefix##BaseRegistry)
