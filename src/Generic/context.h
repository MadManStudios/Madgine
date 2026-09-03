#pragma once

namespace Engine {

template <typename T>
struct Contextual {
    Contextual(T t)
        : mValue(std::forward<T>(t))
    {
    }

    operator T &()
    {
        return mValue;
    }

    auto operator->()
    {
        if constexpr (std::is_pointer_v<T>) {
            return mValue;
        } else {
            return &mValue;
        }
    }

    T mValue;
};

}