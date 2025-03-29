#pragma once

namespace Engine {
namespace Serialize {

    struct FilterCategory;

    struct DefaultFilter {
        using Category = FilterCategory;

        template <typename T>
        static bool filter(const T &)
        {
            return true;
        }
    };

    template <auto f>
    struct CustomFilter {
        using Category = FilterCategory;

        template <typename T>
        static bool filter(const T &t)
        {
            return f(t);
        }
    };

    template <typename... Configs>
    using FilterSelector = ConfigSelectorDefault<FilterCategory, DefaultFilter, Configs...>;

}
}