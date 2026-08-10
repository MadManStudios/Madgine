#pragma once

namespace Engine {

template <typename T>
struct Contextual {
    Contextual(T t)
        : mValue(std::forward<T>(t))
    {
    }

    operator T &()
    {
        return mValue;
    }

    std::remove_reference_t<T> *operator->()
    {
        return &mValue;
    }

    T mValue;
};

template <typename T>
struct context_get_t {

    template <typename Context>
        requires(is_tag_invocable_v<context_get_t<T>, Context>)
    T *operator()(Context &&context) const
        noexcept(is_nothrow_tag_invocable_v<context_get_t<T>, Context>)
    {
        return tag_invoke(*this, std::forward<Context>(context));
    }
};

template <typename T>
inline constexpr context_get_t<T> context_get;

struct context_set_t {

    template <typename Context, typename T>
        requires(is_tag_invocable_v<context_set_t, Context, T &>)
    auto operator()(Context &&context, T &t) const
        noexcept(is_nothrow_tag_invocable_v<context_set_t, Context, T &>)
            -> tag_invoke_result_t<context_set_t, Context, T &>
    {
        return tag_invoke(*this, std::forward<Context>(context), t);
    }
};

inline constexpr context_set_t context_set;

}