#pragma once

#if __cpp_lib_int_pow2 < 202002L
namespace std {

template <class T>
constexpr int bit_width(T x) noexcept {
    return std::numeric_limits<T>::digits - std::countl_zero(x);
}

}
#endif