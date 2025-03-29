#pragma once

#include "scopeptr.h"

namespace Engine {

template <typename Base = void>
struct VirtualScopeBase : Base {
    virtual ScopePtr customScopePtr() = 0;
};

template <>
struct VirtualScopeBase<void> {
    virtual ScopePtr customScopePtr() = 0;
};

template <typename T, typename Base = VirtualScopeBase<>>
struct VirtualScope : Base {
    using Base::Base;
    virtual ScopePtr customScopePtr() override 
    {
        return { this, table<decayed_t<T>> };
    }
};

template struct META_EXPORT VirtualScopeBase<void>;

}
