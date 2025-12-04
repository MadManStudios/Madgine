#pragma once

#include "Generic/nulledptr.h"
#include "Generic/replace.h"

namespace Engine {
namespace Scene {
    namespace Entity {

        template <typename T>
        struct EntityComponentFreeListConfig {

            static bool isFree(ManualLifetime<T> &data)
            {
                return data.unsafeAccess().isFree();
            }

            static uintptr_t *getLocation(ManualLifetime<T> &data)
            {
                static_assert(sizeof(T) >= 2 * sizeof(uintptr_t));
                return (&reinterpret_cast<uintptr_t &>(data)) + 1;
            }
        };

    }
}
}