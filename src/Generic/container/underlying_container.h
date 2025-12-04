#pragma once

namespace Engine {

template <typename C, typename = void>
struct underlying_container;

template <typename C, typename = void>
struct base_container {
    typedef C type;
};

template <typename C>
struct base_container<C, std::void_t<typename underlying_container<C>::type>> : base_container<typename underlying_container<C>::type> {
};

}