#pragma once

#include "Meta/reflect/ref.h"

namespace Engine {
namespace Behavior {

    struct get_named_d_t {
        using signature = Reflect::Result(std::string_view, Reflect::ValueRef);

        template <typename T>
            requires(!is_tag_invocable_v<get_named_d_t, T &, std::string_view, Reflect::ValueRef>)
        auto operator()(T &t, std::string_view name, Reflect::ValueRef out) const
        {
            return REFLECT_UNKNOWN_ERROR() << "Named value '" << name << "' not found";
        }

        template <typename T>
            requires(is_tag_invocable_v<get_named_d_t, T &, std::string_view, Reflect::ValueRef>)
        auto operator()(T &t, std::string_view name, Reflect::ValueRef out) const
            noexcept(is_nothrow_tag_invocable_v<get_named_d_t, T &, std::string_view, Reflect::ValueRef>)
                -> tag_invoke_result_t<get_named_d_t, T &, std::string_view, Reflect::ValueRef>
        {
            return tag_invoke(*this, t, name, out);
        }
    };

    inline constexpr get_named_d_t get_named_d;

}
}