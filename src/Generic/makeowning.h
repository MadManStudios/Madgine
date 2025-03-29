#pragma once

namespace Engine {

template <typename T>
struct MakeOwning {
    static T apply(T &&t)
    {
        return std::forward<T>(t);
    }

    using type = T;
};

template <>
struct MakeOwning<void> {
    using type = void;
};

template <typename T>
struct MakeOwning<T &> : MakeOwning<T> {
};

template <typename T>
struct MakeOwning<const T> : MakeOwning<T> {
};

template <StringViewable S>
struct MakeOwning<S> {
    static std::string apply(const S &s)
    {
        return s;
    }
    using type = std::string;
};

template <typename T>
using MakeOwning_t = typename MakeOwning<T>::type;

}