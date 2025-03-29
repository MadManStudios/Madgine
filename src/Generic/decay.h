#pragma once

namespace Engine {

namespace __generic_impl__ {
    template <typename T, typename = void>
    struct decayBase {
        using type = void;
    };

    template <typename T>
    struct decayBase<T, std::void_t<typename T::decay_t>> {
        using type = typename T::decay_t;
    };

    template <typename T>
    struct decayBase<T&, std::void_t<typename T::decay_t>> {
        using type = typename T::decay_t&;
    };

    template <typename T, T v>
    struct decayBase<std::integral_constant<T, v>, void> {
        using type = T;
    };
}

template <typename T>
using decay_t = typename __generic_impl__::decayBase<T>::type;

template <typename T, typename Decay>
struct decayed {
    using type = typename decayed<Decay, decay_t<Decay>>::type;
};

template <typename T>
struct decayed<T, void> {
    using type = T;
};

template <typename T>
using decayed_t = typename decayed<T, decay_t<T>>::type;

template <typename T>
decltype(auto) decay(T&& item) {
    return std::forward<decayed_t<T>>(item);
}

}