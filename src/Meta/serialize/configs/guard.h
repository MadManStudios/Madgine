#pragma once

#include "Generic/delayedconstruct.h"

namespace Engine {
namespace Serialize {

    struct GuardCategory;

    template <typename... Guards>
    struct Guard {
        using Category = GuardCategory;

        template <typename Context>
        static auto guard(Context &context) -> std::tuple<decltype(Guards::guard(context))...>
        {
            return std::make_tuple(DelayedConstruct { [&]() { return Guards::guard(context); } }...);
        }
    };

    template <auto Guard>
    struct CallableGuard {
        using Category = GuardCategory;

        template <typename Context>
        static decltype(auto) guard(Context &&context)
        {
            return TupleUnpacker::invoke(Guard, context);
        }
    };

    template <typename... Configs>
    using GuardSelector = typename ConfigGroupSelector<GuardCategory, Configs...>::template instantiate<Guard>;

}
}