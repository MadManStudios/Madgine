#pragma once

#include "callable_traits.h"

namespace Engine {

template <typename CPO, typename R, std::same_as<void> T, typename... Args>
struct tag_invocable_view_helper {

    tag_invocable_view_helper()
        : tag_invocable_view_helper(Void {})
    {
    }

    template <Concepts::DecayedNoneOf<tag_invocable_view_helper> U>
        requires std::invocable<CPO, U &, Args...>
    tag_invocable_view_helper(U &&t)
        : mPtr(std::addressof(t))
        , mF([](const void *p, Args... args) -> R {
            U &t = *static_cast<std::remove_reference_t<U> *>(const_cast<void *>(p));
            return CPO {}(t, std::forward<Args>(args)...);
        })
    {
    }

    template <Concepts::DecayedOneOf<tag_invocable_view_helper> U>
    friend R tag_invoke(CPO, U &&self, Args... args)
    {
        return self.mF(self.mPtr, std::forward<Args>(args)...);
    }

    const void *mPtr = nullptr;
    R (*mF)(const void *, Args...) = nullptr;
};

template <typename CPO, typename Signature = CPO::signature>
using tag_invocable_view = CallableTraits<Signature>::template instance<tag_invocable_view_helper, CPO>;

}