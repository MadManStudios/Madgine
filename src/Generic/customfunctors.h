#pragma once

#include "memberoffsetptr.h"

namespace Engine {

template <auto F>
struct MemberFunctor {
    template <typename T, typename... Args>
    decltype(auto) operator()(T *t, Args &&...args)
    {
        return (t->*F)(std::forward<Args>(args)...);
    }
};

template <auto F>
struct UnpackingMemberFunctor {
    template <typename T, typename... Args>
    decltype(auto) operator()(T *t, Args &&...args)
    {
        return TupleUnpacker::invoke(F, t, std::forward<Args>(args)...);
    }
};

template <auto f, typename OffsetPtr = TaggedPlaceholder<MemberOffsetPtrTag, 0>>
struct ParentFunctor {
    template <typename... Args>
    decltype(auto) operator()(Args &&...args)
    {
        return (OffsetPtr::parent(this)->*f)(std::forward<Args>(args)...);
    }
};

}