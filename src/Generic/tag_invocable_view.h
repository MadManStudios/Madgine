#pragma once

#include "callable_traits.h"

namespace Engine {

template <typename CPO, typename R, std::same_as<void> T, std::same_as<Placeholder<0>> Placeholder, typename... Args>
struct tag_invocable_view_helper {

    tag_invocable_view_helper() = default;

    template <typename U>
        requires tag_invocable<CPO, U &, Args...>
    tag_invocable_view_helper(U &&t)
        : mPtr(std::addressof(t))
        , mF([](const void *p, Args...args) -> R {
            U &t = *static_cast<std::remove_reference_t<U> *>(const_cast<void*>(p));
            return CPO {}(t, std::forward<Args>(args)...);
        })
    {
    }

    template <Concepts::DecayedOneOf<tag_invocable_view_helper> U>
    friend R tag_invoke(CPO, U &&self, Args...args)
    {
        return self.mF(self.mPtr, std::forward<Args>(args)...);
    }

    const void *mPtr = nullptr;
    R (*mF)(const void *, Args...) = nullptr;
};

template <typename CPO>
using tag_invocable_view = CallableTraits<typename CPO::signature>::template instance<tag_invocable_view_helper, CPO>;

}