#pragma once

namespace Engine {
namespace Serialize {

    template <typename T>
    decltype(auto) physical(T &t)
    {
        if constexpr (requires { t.physical(); })
            return t.physical();
        else
            return t;
    }
}
}