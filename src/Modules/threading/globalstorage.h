#pragma once

#include "Generic/proxy.h"

namespace Engine {
namespace Threading {

    struct GlobalStorage {

        template <typename T>
        using container_type = Proxy<T, false>;
    };

}
}