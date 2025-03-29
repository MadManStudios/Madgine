#pragma once

namespace Engine {
	
template <typename Cmp>
struct comparator_traits : Cmp::traits {};


template <typename T>
struct comparator_traits<std::less<T>> {
    using type = T;
    using item_type = T;

    static const T &to_cmp_type(const T &t)
    {
        return t;
    }
};



}