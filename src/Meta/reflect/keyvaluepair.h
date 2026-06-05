#pragma once

#include "value.h"

namespace Engine {
namespace Reflect {

    struct KeyValuePair {

        template <size_t I>
        Value &&get() &&
        {
            if constexpr (I == 0)
                return std::move(mKey);
            else if constexpr (I == 1)
                return std::move(mValue);
            else
                static_assert(dependent_bool<size_t, (I < 2)>::value, "Index out of bounds for KeyValuePair");
        }

        template <size_t I>
        const Value &&get() const &&
        {
            if constexpr (I == 0)
                return std::move(mKey);
            else if constexpr (I == 1)
                return std::move(mValue);
            else
                static_assert(dependent_bool<size_t, (I < 2)>::value, "Index out of bounds for KeyValuePair");
        }

        template <size_t I>
        Value &get() &
        {
            if constexpr (I == 0)
                return mKey;
            else if constexpr (I == 1)
                return mValue;
            else
                static_assert(dependent_bool<size_t, (I < 2)>::value, "Index out of bounds for KeyValuePair");
        }

        template <size_t I>
        const Value &get() const &
        {
            if constexpr (I == 0)
                return mKey;
            else if constexpr (I == 1)
                return mValue;
            else
                static_assert(dependent_bool<size_t, (I < 2)>::value, "Index out of bounds for KeyValuePair");
        }

        Value mKey;
        Value mValue;
    };

}
}

namespace std {
template <>
struct tuple_size<Engine::Reflect::KeyValuePair> : std::integral_constant<size_t, 2> { };

template <size_t I>
struct tuple_element<I, Engine::Reflect::KeyValuePair> {
    using type = Engine::Reflect::Value;
};

}
