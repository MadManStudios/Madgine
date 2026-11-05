#pragma once

namespace Engine {
namespace Containers {

    template <typename T>
    std::vector<std::ranges::range_value_t<T>> safeIterate(T &&t)
    {
        return { t.begin(), t.end() };
    }

}
}