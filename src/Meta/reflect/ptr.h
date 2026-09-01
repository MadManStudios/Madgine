#pragma once

#include "scopeptr.h"

namespace Engine {
namespace Reflect {

    template <typename T>
    struct Pointer : ScopePtr {
        using type = T;

        Pointer(ScopePtr ptr)
            : ScopePtr(std::move(ptr))
        {
        }
    };

}
}