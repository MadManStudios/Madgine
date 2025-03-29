#pragma once

namespace Engine {

template <typename T>
struct FreeBitCount {
    static constexpr size_t value = 0;
};

template <typename T>
concept has_FreeBitCount = requires
{
    T::FreeBitCount;
};

template <has_FreeBitCount T>
struct FreeBitCount<T> {
    static constexpr size_t value = T::FreeBitCount;
};

template <>
struct FreeBitCount<std::monostate> {
    static constexpr size_t value = 8;
};

template <typename T>
constexpr size_t FreeBitCount_v = FreeBitCount<T>::value;

}