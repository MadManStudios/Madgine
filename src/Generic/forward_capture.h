#pragma once

namespace Engine {

template <typename T>
struct CaptureHelper {
    template <typename... Args>
    decltype(auto) operator()(Args &&...args) const
    {
        return mT(std::forward<Args>(args)...);
    }
    operator T &()
    {
        return mT;
    }
    T mT;
};

template <typename T>
CaptureHelper<T> forward_capture(T &&t)
{
    return { std::forward<T>(t) };
}

}