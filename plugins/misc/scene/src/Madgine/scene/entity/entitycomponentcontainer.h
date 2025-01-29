#pragma once

#include "Generic/container/multicontainer.h"
#include "Generic/nulledptr.h"
#include "Generic/replace.h"

namespace Engine {
namespace Scene {
    namespace Entity {

        struct get_component_t {
            friend decltype(auto) tag_invoke(get_component_t, auto &&item)
            {
                return get<0>(std::forward<decltype(item)>(item));
            }

            template <typename T>
            auto operator()(T &&item) const
                noexcept(is_nothrow_tag_invocable_v<get_component_t, T>)
                    -> tag_invoke_result_t<get_component_t, T>
            {
                return tag_invoke(*this, std::forward<T>(item));
            }
        };

        inline constexpr get_component_t get_component;

        struct get_entity_ptr_t {
            friend decltype(auto) tag_invoke(get_entity_ptr_t, auto &&item)
            {
                return get<1>(std::forward<decltype(item)>(item));
            }

            template <typename T>
            auto operator()(T &&item) const
                noexcept(is_nothrow_tag_invocable_v<get_entity_ptr_t, T>)
                    -> tag_invoke_result_t<get_entity_ptr_t, T>
            {
                return tag_invoke(*this, std::forward<T>(item));
            }
        };

        inline constexpr get_entity_ptr_t get_entity_ptr;

        MADGINE_SCENE_EXPORT void relocateEntityComponent(Entity *entity, EntityComponentHandle<EntityComponentBase> index);

        template <template <typename...> typename C, typename T = Placeholder<0>>
        using EntityComponentContainerImpl = MultiContainer<C, T, NulledPtr<Entity>>;

        template <typename T>
        struct EntityComponentFreeListConfig {

            static bool isFree(ReferenceTuple<ManualLifetime<T>, ManualLifetime<NulledPtr<Entity>>> data)
            {
                return !get<1>(data);
            }

            static uintptr_t *getLocation(ReferenceTuple<ManualLifetime<T>, ManualLifetime<NulledPtr<Entity>>> data)
            {
                static_assert(sizeof(T) >= sizeof(uintptr_t));
                return &reinterpret_cast<uintptr_t &>(get<0>(data));
            }
        };

        struct EntityComponentRelocateFunctor {
            template <typename It>
            void operator()(const It &it, const It &begin)
            {
                using T = std::remove_reference_t<decltype(get_component(*it))>;
                relocateEntityComponent(get_entity_ptr(*it), EntityComponentHandle<T> { static_cast<uint32_t>(std::distance(begin, it)) });
            }
        };

    }
}
}