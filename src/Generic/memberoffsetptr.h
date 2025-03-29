#pragma once

#include "replace.h"
#include "offsetptr.h"

namespace Engine {

struct MemberOffsetPtrTag;

template <typename T, typename M>
struct MemberOffsetPtr {

    using member_type = M;
    using parent_type = T;

    MemberOffsetPtr() = default;

    template <typename Target>
    MemberOffsetPtr(Target(T::*P))
        : mOffset(reinterpret_cast<uintptr_t>(&static_cast<M &>(static_cast<T *>(reinterpret_cast<void *>(0x1))->*P)) - 1)
    {        
    }

    T *parent(M *m)
    {
        return reinterpret_cast<T *>(reinterpret_cast<std::byte *>(m) - mOffset);
    }

    const T *parent(const M *m)
    {
        return reinterpret_cast<const T *>(reinterpret_cast<const std::byte *>(m) - mOffset);
    }

    template <typename _T, typename _M = M>
    OffsetPtr offset()
    {
        OffsetPtr offset = mOffset;
        offset += reinterpret_cast<uintptr_t>(static_cast<T *>(static_cast<_T *>(reinterpret_cast<void *>(0x1)))) - 1;
        offset += reinterpret_cast<uintptr_t>(static_cast<_M *>(static_cast<M *>(reinterpret_cast<void *>(0x1)))) - 1;
        return offset;
    }

    OffsetPtr mOffset;
};

template <typename P>
struct MemberOffsetPtrWrapper {

    using ptr_type = decltype(P::value);
    using member_type = typename ptr_type::member_type;
    using parent_type = typename ptr_type::parent_type;

    template <typename Ty>
    static parent_type *parent(Ty *child)
    {
        return P::value.parent(static_cast<member_type *>(child));
    }

    template <typename Ty>
    static const parent_type *parent(const Ty *child)
    {
        return P::value.parent(static_cast<const member_type *>(child));
    }

    template <typename _T = parent_type, typename _M = member_type>
    static OffsetPtr offset()
    {
        return P::value.template offset<_T, _M>();
    }
};

#define DECLARE_MEMBER_OFFSET(Name) \
    struct __access_##Name##__;

#define DEFINE_MEMBER_OFFSET(Name)                                                                   \
    static ::Engine::MemberOffsetPtr<Self, decltype(Name)> __##Name##_value__()                     \
    {                                                                                         \
        return &Self::Name;                                                                   \
    }                                                                                         \
    struct __access_##Name##__ {                                                              \
        static inline ::Engine::MemberOffsetPtr<Self, decltype(Name)> value = __##Name##_value__(); \
    };

#define MEMBER_OFFSET_CONTAINER(Name, Init, ...)                                                                                                \
    DECLARE_MEMBER_OFFSET(Name)                                                                                                           \
    ::Engine::replace<__VA_ARGS__>::tagged<::Engine::MemberOffsetPtrTag, ::Engine::MemberOffsetPtrWrapper<__access_##Name##__>> Name Init; \
    DEFINE_MEMBER_OFFSET(Name)

}