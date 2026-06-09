#pragma once

#include "Generic/execution/query.h"
#include "Generic/execution/concepts.h"

namespace Engine {
namespace Debug {

    struct get_debug_context_t {

        using signature = ContextInfo &();

        template <typename T>
            requires tag_invocable<get_debug_context_t, T &>
        auto operator()(T &t) const
            noexcept(is_nothrow_tag_invocable_v<get_debug_context_t, T &>)
                -> tag_invoke_result_t<get_debug_context_t, T &>
        {
            return tag_invoke(*this, t);
        }
    };

    inline constexpr get_debug_context_t get_debug_context;

    inline constexpr auto with_debug_context = [](ContextInfo &info) { return Execution::with_query_value(get_debug_context, info); };

}
}