#pragma once

#include "Generic/nulledptr.h"
#include "Generic/replace.h"

namespace Engine {
namespace Scene {
    namespace Entity {

        MADGINE_SCENE_EXPORT void relocateEntityComponent(Entity *entity, EntityComponentHandle<EntityComponentBase> index);

        template <typename T>
        struct EntityComponentFreeListConfig {

            static bool isFree(ManualLifetime<T> &data)
            {
                return !data.unsafeAccess().entity();
            }

            static uintptr_t *getLocation(ManualLifetime<T> &data)
            {
                static_assert(sizeof(T) >= 2 * sizeof(uintptr_t));
                return (& reinterpret_cast<uintptr_t &>(data)) + 1;
            }
        };

        struct EntityComponentRelocateFunctor {
            template <typename It>
            void operator()(const It &it, const It &begin)
            {
                using T = std::remove_reference_t<decltype(*it)>;
                relocateEntityComponent(it->entity(), EntityComponentHandle<T> { static_cast<uint32_t>(std::distance(begin, it)) });
            }
        };

    }
}
}