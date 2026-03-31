#pragma once

namespace Engine {

namespace __generic_impl__ {
    template <typename T, typename = void>
    struct decayBase {
        using type = void;
    };

    template <typename T>
    struct decayBase<T, std::void_t<typename T::meta_t>> {
        using type = typename T::meta_t;
    };

    template <typename T>
    struct decayBase<T &, std::void_t<typename T::meta_t>> {
        using type = typename T::meta_t &;
    };

    template <typename T, T v>
    struct decayBase<std::integral_constant<T, v>, void> {
        using type = T;
    };
}

template <typename T>
using meta_decay_t = typename __generic_impl__::decayBase<T>::type;

template <typename T, typename Decay>
struct meta_decayed {
    using type = typename meta_decayed<Decay, meta_decay_t<Decay>>::type;
};

template <typename T>
struct meta_decayed<T, void> {
    using type = T;
};

template <typename T>
struct meta_decayed<T, T> {
    using type = T;
};

template <typename T>
using meta_decayed_t = typename meta_decayed<T, meta_decay_t<T>>::type;

}