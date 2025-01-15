#pragma once

#include "lift.h"

namespace Engine {

template <typename F, typename... Args>
struct PipableFromRight {
    template <typename Ty>
    requires std::invocable<F, Ty, Args...>
    friend decltype(auto) operator|(Ty &&ty, PipableFromRight &&p)
    {
        return p.call(std::forward<Ty>(ty), std::make_index_sequence<sizeof...(Args)> {});
    }

    template <typename Ty>
    requires std::invocable<F, Ty, Args...>
    decltype(auto) operator()(Ty &&ty) &&
    {
        return call(std::forward<Ty>(ty), std::make_index_sequence<sizeof...(Args)> {});
    }

    template <typename Ty, size_t... I>
    decltype(auto) call(Ty&& ty, std::index_sequence<I...>) {
        return std::forward<F>(mF)(std::forward<Ty>(ty), std::get<I>(std::move(mArgs))...);
    }

    F mF;
    std::tuple<Args...> mArgs;
};

template <typename F, typename... Args>
PipableFromRight<F, Args...> pipable_from_right(F &&f, Args &&...args)
{
    return { std::forward<F>(f), { std::forward<Args>(args)... } };
}

#define MAKE_PIPABLE_FROM_RIGHT(f)                                       \
    template <typename... Args>                               \
    auto f(Args &&...args)                                    \
    {                                                         \
        return ::Engine::pipable_from_right(LIFT(f), std::forward<Args>(args)...); \
    }


template <typename F, typename... Args>
struct PipableFromLeft {
    template <typename Ty>
    friend decltype(auto) operator|(PipableFromLeft &&p, Ty &&ty)
    {
        return p.call(std::forward<Ty>(ty), std::make_index_sequence<sizeof...(Args)> {});
    }

    template <typename Ty, size_t... I>
    decltype(auto) call(Ty &&ty, std::index_sequence<I...>)
    {
        return std::forward<F>(mF)(std::get<I>(std::move(mArgs))..., std::forward<Ty>(ty));
    }

    F mF;
    std::tuple<Args...> mArgs;
};

template <typename F, typename... Args>
PipableFromLeft<F, Args...> pipable_from_left(F &&f, Args &&...args)
{
    return { std::forward<F>(f), { std::forward<Args>(args)... } };
}

#define MAKE_PIPABLE_FROM_LEFT(f)                                                 \
    template <typename... Args>                                                    \
    auto f(Args &&...args)                                                         \
    {                                                                              \
        return ::Engine::pipable_from_left(LIFT(f), std::forward<Args>(args)...); \
    }

}