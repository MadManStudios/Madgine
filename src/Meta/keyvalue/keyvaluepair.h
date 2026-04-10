#pragma once

#include "valuetype.h"

namespace Engine {

struct KeyValuePair {

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

    ValueType mKey;
    ValueType mValue;
};

}

namespace std {
template <>
struct tuple_size<Engine::KeyValuePair> : std::integral_constant<size_t, 2> { };

/* template <typename V1, typename... V>
struct tuple_element<0, Engine::Execution::ValueStorageImpl<V1, V...>> {
    using type = V1;
};
template <size_t I, typename V1, typename... V>
struct tuple_element<I, Engine::Execution::ValueStorageImpl<V1, V...>> : tuple_element<I - 1, Engine::Execution::ValueStorageImpl<V...>> { };*/

}