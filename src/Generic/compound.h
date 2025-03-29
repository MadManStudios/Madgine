#pragma once

namespace Engine {

template <typename T>
using ctor_t = typename T::ctor;

template <typename... Ty>
struct Compound : Ty... {
    Compound() = default;
    Compound(ctor_t<Ty>... args)
        : Ty({ args })...
    {
    }

    template <typename T>
    static const constexpr bool holds = type_pack<Ty...>::template contains<T>;
};

}