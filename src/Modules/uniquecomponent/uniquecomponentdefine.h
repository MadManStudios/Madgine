#pragma once

#include "Generic/replace.h"
#include "annotations.h"

//base is included in __VA_ARGS__ to circumvent the problem with empty __VA_ARGS__ and ,
#define DECLARE_UNIQUE_COMPONENT2(ns, prefix, registry, component, /*base, */...)                                                                                                 \
    namespace ns {                                                                                                                                                                \
    using prefix##Registry = registry<__VA_ARGS__>;                                                                                                                               \
    using prefix##BaseRegistry = Engine::UniqueComponent::Registry<__VA_ARGS__>;                                                                                                  \
    using prefix##Collector = Engine::UniqueComponent::Collector<prefix##Registry>;                                                                                  \
    template <typename C>                                                                                                                                                         \
    using prefix##Container = Engine::UniqueComponent::Container<typename Engine::replace<C>::template type<std::unique_ptr<FIRST(__VA_ARGS__)>>, prefix##Registry>; \
    using prefix##Selector = Engine::UniqueComponent::Selector<prefix##Registry>;                                                                                    \
    template <typename T, typename Base = FIRST(__VA_ARGS__)>                                                                                                                     \
    using prefix##Component = component<Engine::UniqueComponent::Component<T, prefix##Collector, Base>>;                                                                          \
    template <typename T, typename Base = FIRST(__VA_ARGS__)>                                                                                                                     \
    using prefix##VirtualBase = component<Engine::UniqueComponent::VirtualComponentBase<T, prefix##Collector, Base>>;                                                             \
    template <typename T, typename Base>                                                                                                                                          \
    using prefix##VirtualImpl = Engine::UniqueComponent::VirtualComponentImpl<T, Base>;                                                                                \
    }                                                                                                                                                                             \
    REGISTER_TYPE(ns::prefix##Registry)

#define DECLARE_UNIQUE_COMPONENT(ns, prefix, /*base, */...) DECLARE_UNIQUE_COMPONENT2(ns, prefix, Engine::UniqueComponent::Registry, std::type_identity_t, __VA_ARGS__)

#define DECLARE_NAMED_UNIQUE_COMPONENT(ns, prefix, /*base, */...)                                                                       \
    DECLARE_UNIQUE_COMPONENT2(ns, prefix, Engine::UniqueComponent::NamedRegistry, Engine::UniqueComponent::NamedComponent, __VA_ARGS__) \
    REGISTER_TYPE(ns::prefix##BaseRegistry)

#if defined(STATIC_BUILD)
#    define EXPORT_REGISTRY(Registry, BaseRegistry)
#else
#    define EXPORT_REGISTRY(Registry, BaseRegistry) DLL_EXPORT_VARIABLE3_ORDER(, BaseRegistry, Registry, Engine::UniqueComponent::, registry, , {}, BaseRegistry)
#endif

#define DEFINE_UNIQUE_COMPONENT(ns, prefix) \
    EXPORT_REGISTRY(ns::prefix##Registry, ns::prefix##BaseRegistry)
