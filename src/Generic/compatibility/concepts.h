#pragma once

#if __cpp_lib_concepts < 201907L

namespace std {
template <typename T, typename U>
concept constructible_from = is_constructible_v<T, U>;
template <typename T, typename U>
concept convertible_to = is_convertible_v<T, U>;
template <typename T, typename U>
concept same_as = is_same_v<T, U>;
template <typename T, typename U>
concept derived_from = is_base_of_v<U, T>;
}

#endif
