#pragma once

#include "Generic/delayedconstruct.h"

namespace Engine {
namespace Serialize {

    struct GuardCategory;

    template <typename... Guards>
    struct Guard {
        using Category = GuardCategory;

        static auto guard(const CallerHierarchyBasePtr &hierarchy) -> std::tuple<decltype(Guards::guard(hierarchy))...>
        {
            return std::make_tuple(DelayedConstruct<decltype(Guards::guard(hierarchy))> { [&]() { return Guards::guard(hierarchy); } }...);
        }
    };

    template <auto Guard>
    struct CallableGuard {
        using Category = GuardCategory;

        static decltype(auto) guard(const CallerHierarchyBasePtr &hierarchy)
        {
            return TupleUnpacker::invoke(Guard, hierarchy);
        }
    };

    template <typename... Configs>
    using GuardSelector = typename ConfigGroupSelector<GuardCategory, Configs...>::template instantiate<Guard>;

}
}