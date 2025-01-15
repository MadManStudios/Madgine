#pragma once

#include "scopeptr.h"

namespace Engine {

struct META_EXPORT ProxyScopeBase {
    inline virtual ~ProxyScopeBase() = default;
    virtual ScopePtr proxyScopePtr() = 0;
};

template <typename T>
struct ScopeWrapper : ProxyScopeBase, T {
    template <typename... Args>
    ScopeWrapper(Args &&... args)
        : T(std::forward<Args>(args)...)
    {
    }
    virtual ScopePtr proxyScopePtr() override
    {
        return static_cast<T*>(this);
    }
};

}
