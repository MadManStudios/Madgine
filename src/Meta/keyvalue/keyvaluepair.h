#pragma once

#include "valuetype.h"

namespace Engine {

struct KeyValuePair {

    template <size_t I>
    ValueType &&get() &&
    {
        if constexpr (I == 0)
            return std::move(mKey);
        else if constexpr (I == 1)
            return std::move(mValue);
        else
            static_assert(dependent_bool<size_t, (I < 2)>::value, "Index out of bounds for KeyValuePair");
    }

    template <size_t I>
    const ValueType &&get() const &&
    {
        if constexpr (I == 0)
            return std::move(mKey);
        else if constexpr (I == 1)
            return std::move(mValue);
        else
            static_assert(dependent_bool<size_t, (I < 2)>::value, "Index out of bounds for KeyValuePair");
    }

    template <size_t I>
    ValueType &get() &
    {
        if constexpr (I == 0)
            return mKey;
        else if constexpr (I == 1)
            return mValue;
        else
            static_assert(dependent_bool<size_t, (I < 2)>::value, "Index out of bounds for KeyValuePair");
    }

    template <size_t I>
    const ValueType &get() const &
    {
        if constexpr (I == 0)
            return mKey;
        else if constexpr (I == 1)
            return mValue;
        else
            static_assert(dependent_bool<size_t, (I < 2)>::value, "Index out of bounds for KeyValuePair");
    }

    ValueType mKey;
    ValueType mValue;
};

}

namespace std {
template <>
struct tuple_size<Engine::KeyValuePair> : std::integral_constant<size_t, 2> { };

template <size_t I>
struct tuple_element<I, Engine::KeyValuePair> {
    using type = Engine::ValueType;
};

}