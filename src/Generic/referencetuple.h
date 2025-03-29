#pragma once

#include "manuallifetime.h"

namespace Engine {

template <typename... Ty>
struct ReferenceTuple {

    ReferenceTuple(std::tuple<Ty &...> data)
        : mData(data)
    {
    }

    template <typename... Ty2>
    requires(std::convertible_to<Ty2 &, Ty &> &&...)
        ReferenceTuple(ReferenceTuple<Ty2...> data)
        : mData(static_cast<std::tuple<Ty2&...>>(data))
    {
    }

    operator std::tuple<Ty &...>()
    {
        return mData;
    }

    template <size_t I>
    friend decltype(auto) get(ReferenceTuple &data)
    {
        return std::get<I>(data.mData);
    }

    template <size_t I>
    friend decltype(auto) get(const ReferenceTuple &data)
    {
        return std::get<I>(data.mData);
    }

    template <typename T>
    friend decltype(auto) get(ReferenceTuple &data)
    {
        return std::get<T &>(data.mData);
    }

    template <typename T>
    friend decltype(auto) get(const ReferenceTuple &data)
    {
        return std::get<T>(data.mData);
    }

    std::tuple<Ty *...> operator&() const
    {
        return {
            { &std::get<Ty &>(mData)... }
        };
    }

    friend void swap(const ReferenceTuple &first, const ReferenceTuple &second)
    {
        (std::swap(std::get<Ty &>(first.mData), std::get<Ty &>(second.mData)), ...);
    }

    friend auto tag_invoke(destruct_t, ReferenceTuple tuple)
    {
        (destruct(std::get<Ty &>(tuple.mData)), ...);
    }

    template <typename... Args>
    friend auto tag_invoke(construct_t, ReferenceTuple tuple, Args&&... args)
    {
        (construct(std::get<Ty &>(tuple.mData), std::forward<Args>(args)), ...);
    }

    template <typename... Tuples>
    friend auto tag_invoke(construct_t, ReferenceTuple tuple, std::piecewise_construct_t, Tuples &&...tuples)
    {
        (TupleUnpacker::invokeFromTuple(construct, std::get<Ty &>(tuple.mData), std::forward<Tuples>(tuples)), ...);
    }

    

private:
    std::tuple<Ty &...> mData;
};

}

namespace std {

template <typename... Ty>
struct tuple_size<Engine::ReferenceTuple<Ty...>> : std::integral_constant<size_t, sizeof...(Ty)> {
};

template <size_t I, typename... Ty>
struct tuple_element<I, Engine::ReferenceTuple<Ty...>> {
    using type = typename Engine::type_pack<Ty...>::template select<I> &;
};

}