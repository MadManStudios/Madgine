#pragma once

#include "callable_traits.h"
#include "defaultassign.h"

namespace Engine {
namespace TupleUnpacker {

    template <typename T>
    decltype(auto) ensureTuple(T &&value)
    {
        if constexpr (InstanceOf<std::decay_t<T>, std::tuple>)
            return T { std::forward<T>(value) };
        else
            return std::make_tuple(std::forward<T>(value));
    }

    template <typename T, typename... Ts, size_t... Is>
    auto prependImpl(T &&val, std::tuple<Ts...> &&tuple, std::index_sequence<Is...>)
    {
        return std::tuple<T, Ts...> { std::forward<T>(val), std::get<Is>(std::move(tuple))... };
    }

    template <typename T, typename... Ts, size_t... Is>
    auto prependImpl(T &&val, const std::tuple<Ts...> &tuple, std::index_sequence<Is...>)
    {
        return std::tuple<T, Ts...> { std::forward<T>(val), std::get<Is>(tuple)... };
    }

    template <typename T, typename Tuple>
    auto prepend(T &&val, Tuple &&tuple)
    {

        return prependImpl(std::forward<T>(val), std::forward<Tuple>(tuple),
            std::make_index_sequence<std::tuple_size_v<Tuple>>());
    }

    template <typename T, typename... Ts, size_t... Is>
    auto popFrontImpl(std::tuple<T, Ts...> &&tuple, std::index_sequence<Is...>)
    {
        return std::tuple<Ts...> { std::get<1 + Is>(std::move(tuple))... };
    }

    template <typename T, typename... Ts, size_t... Is>
    auto popFrontImpl(const std::tuple<T, Ts...> &tuple, std::index_sequence<Is...>)
    {
        return std::tuple<Ts...> { std::get<1 + Is>(tuple)... };
    }

    template <typename Tuple>
    auto popFront(Tuple &&tuple)
    {

        return popFrontImpl(std::forward<Tuple>(tuple),
            std::make_index_sequence<std::tuple_size_v<Tuple> - 1>());
    }

    void popFront(std::tuple<>) = delete;

    template <typename Tuple>
    Tuple &&shiftTupleReference(Tuple &&tuple)
    {
        return std::forward<Tuple>(tuple);
    }

    template <typename... T, size_t... Is>
    std::tuple<T &...> shiftTupleReference(std::tuple<T...> &tuple, std::index_sequence<Is...>)
    {
        return { std::get<Is>(tuple)... };
    }

    template <typename... T>
    std::tuple<T &...> shiftTupleReference(std::tuple<T...> &tuple)
    {
        return shiftTupleReference(tuple, std::index_sequence_for<T...> {});
    }

    template <size_t I, typename Tuple, size_t... S, size_t... T>
    decltype(auto) expand(Tuple &&tuple, std::index_sequence<S...>, std::index_sequence<T...>)
    {
        return std::tuple_cat(
            std::forward_as_tuple(std::get<S>(std::forward<Tuple>(tuple))...),
            shiftTupleReference(std::get<I>(std::forward<Tuple>(tuple))),
            std::forward_as_tuple(std::get<I + 1 + T>(std::forward<Tuple>(tuple))...));
    }

    template <size_t I, typename Tuple>
    decltype(auto) expand(Tuple &&tuple)
    {
        constexpr size_t S = std::tuple_size_v<Tuple>;
        return expand<I>(
            std::forward<Tuple>(tuple),
            std::make_index_sequence<I>(),
            std::make_index_sequence<S - 1 - I>());
    }

    template <typename Tuple, size_t... Is>
    decltype(auto) flatten(Tuple &&tuple, std::index_sequence<Is...>);

    template <typename NonTuple>
    decltype(auto) flatten(NonTuple &&t)
    {
        return std::forward_as_tuple(std::forward<NonTuple>(t));
    }

    template <Tuple Tuple>
    decltype(auto) flatten(Tuple &&tuple)
    {
        return flatten(std::forward<Tuple>(tuple), std::make_index_sequence<std::tuple_size_v<Tuple>>());
    }

    template <typename Tuple, size_t... Is>
    decltype(auto) flatten(Tuple &&tuple, std::index_sequence<Is...>)
    {
        return std::tuple_cat(flatten(std::get<Is>(std::forward<Tuple>(tuple)))...);
    }

    template <typename F, typename Tuple, size_t... S>
    concept unpackable = std::invocable<F, decltype(std::get<S>(std::declval<Tuple>()))...>;

    template <typename F, typename Tuple, size_t... S>
    requires unpackable<F, Tuple, S...>
    decltype(auto) unpackTuple(F &&f, Tuple &&args, auto_pack<S...>)
    {
        return std::invoke(std::forward<F>(f), std::get<S>(std::forward<Tuple>(args))...);
    }

    template <typename F, typename Tuple>
    requires requires
    {
        unpackTuple(std::declval<F>(), std::declval<Tuple>(),
            make_index_pack<callable_argument_count<F>(std::tuple_size<std::remove_reference_t<Tuple>>::value)>());
    }
    decltype(auto) invokeFromTuple(F &&f, Tuple &&args)
    {
        return unpackTuple(std::forward<F>(f), std::forward<Tuple>(args),
            make_index_pack<callable_argument_count<F>(std::tuple_size<std::remove_reference_t<Tuple>>::value)>());
    }

    template <typename F, typename... Args>
    decltype(auto) invoke(F &&f, Args &&...args)
    {
        return invokeFromTuple(std::forward<F>(f), std::forward_as_tuple(std::forward<Args>(args)...));
    }

    template <typename F, typename... Args>
    decltype(auto) invokeExpand(F &&f, Args &&...args)
    {
        return invokeFromTuple(std::forward<F>(f), expand<sizeof...(args) - 1>(std::forward_as_tuple(std::forward<Args>(args)...)));
    }

    template <typename F, typename... Args>
    decltype(auto) invokeFlatten(F &&f, Args &&...args)
    {
        return invokeFromTuple(std::forward<F>(f), flatten(std::forward_as_tuple(std::forward<Args>(args)...)));
    }

    template <typename R, typename F, typename... Args>
    R invokeDefaultResult(R &&defaultValue, F &&f, Args &&...args)
    {
        using result_t = decltype(TupleUnpacker::invoke(std::forward<F>(f), std::forward<Args>(args)...));
        if constexpr (std::is_convertible_v<result_t, R>) {
            return TupleUnpacker::invoke(std::forward<F>(f), std::forward<Args>(args)...);
        } else {
            TupleUnpacker::invoke(std::forward<F>(f), std::forward<Args>(args)...);
            return std::forward<R>(defaultValue);
        }
    }

    template <typename T, size_t... S, typename Tuple>
    T constructUnpackTuple(Tuple &&args, std::index_sequence<S...>)
    {
        return T { std::get<S>(std::forward<Tuple>(args))... };
    }

    template <typename T, typename Tuple>
    T constructFromTuple(Tuple &&args)
    {
        return constructUnpackTuple<T>(std::forward<Tuple>(args), std::make_index_sequence<std::tuple_size_v<Tuple>>());
    }

    template <typename T, typename... Args>
    T construct(Args &&...args)
    {
        return constructFromTuple<T>(std::forward_as_tuple(std::forward<Args>(args)...));
    }

    template <typename T, typename... Args>
    T constructExpand(Args &&...args)
    {
        return constructFromTuple<T>(expand<sizeof...(args) - 1>(std::forward_as_tuple(std::forward<Args>(args)...)));
    }

    template <typename F>
    auto wrap(F &&f)
    {
        return [f { std::forward<F>(f) }](auto &&...args) mutable { return invoke(f, std::forward<decltype(args)>(args)...); };
    }

    template <typename Tuple, typename F, size_t... Is>
    auto forEach(Tuple &&t, F &&f, std::index_sequence<Is...>)
    {
        return std::tuple { invoke_patch_void(std::forward<F>(f), std::get<Is>(std::forward<Tuple>(t)))... };
    }

    template <typename Tuple, typename F>
    auto forEach(Tuple &&t, F &&f)
    {
        return forEach(std::forward<Tuple>(t), std::forward<F>(f), std::make_index_sequence<std::tuple_size<std::remove_reference_t<Tuple>>::value>());
    }

    template <typename Tuple, typename F, typename T>
    T accumulate(Tuple &&tuple, F &&f, T &&t)
    {
        forEach(std::forward<Tuple>(tuple), [&](auto &&e) {
            t = f(std::forward<decltype(e)>(e), std::forward<T>(t));
        });
        return std::forward<T>(t);
    }

    template <typename Tuple, typename F, size_t... Is>
    decltype(auto) select(Tuple &&t, F &&f, size_t index, std::index_sequence<Is...>)
    {
        using R = std::invoke_result_t<F, decltype(std::get<0>(std::forward<Tuple>(t)))>;
        using Fs = R (*)(Tuple &&, F &&);
        constexpr Fs fs[] = {
            [](Tuple &&t, F &&f) -> R {
                return std::forward<F>(f)(std::get<Is>(std::forward<Tuple>(t)));
            }...
        };
        return fs[index](std::forward<Tuple>(t), std::forward<F>(f));
    }

    template <typename Tuple, typename F>
    decltype(auto) select(Tuple &&t, F &&f, size_t index)
    {
        return select(std::forward<Tuple>(t), std::forward<F>(f), index, std::make_index_sequence<std::tuple_size<std::remove_reference_t<Tuple>>::value>());
    }
}
}