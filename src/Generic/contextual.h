#pragma once

namespace Engine {

template <typename T>
struct Contextual {
    Contextual(T t)
        : mValue(std::forward<T>(t))
    {
    }

    operator T&() {
        return mValue;
    }

    std::remove_reference_t<T>* operator->() {
        return &mValue;
    }

    T mValue;
};

}