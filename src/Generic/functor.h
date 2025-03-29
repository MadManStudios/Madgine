#pragma once

namespace Engine {

struct NoOpFunctor {
    template <typename... Args>
    void operator()(Args &&...) { }
};

template <auto F>
struct Functor {
    template <typename... Args>
    requires std::invocable<decltype(F), Args&&...>
    decltype(auto) operator()(Args &&...args)
    {
        return std::invoke(F, std::forward<Args>(args)...);
    }
};

}