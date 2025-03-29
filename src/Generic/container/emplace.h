#pragma once

#include "container_traits.h"

namespace Engine {

struct emplace_t {

    template <typename T, typename Alloc, typename It, typename... Args>
    requires std::constructible_from<T, Args&&...>
    friend auto tag_invoke(emplace_t, bool &success, std::vector<T, Alloc> &range, const It &where, Args &&...args)
    {
        success = true;
        return range.emplace(where, std::forward<Args>(args)...);
    }

    template <typename T, typename Alloc, typename It, typename... Args>
    friend auto tag_invoke(emplace_t, bool &success, std::list<T, Alloc> &range, const It &where, Args &&...args)
    {
        success = true;
        return range.emplace(where, std::forward<Args>(args)...);
    }

    template <typename T, typename Alloc, typename It, typename... Args>
    friend auto tag_invoke(emplace_t, bool &success, std::deque<T, Alloc> &range, const It &where, Args &&...args)
    {
        success = true;
        return range.emplace(where, std::forward<Args>(args)...);
    }

    template <typename T, typename Cmp, typename Alloc, typename It, typename... Args>
    friend auto tag_invoke(emplace_t, bool &success, std::set<T, Cmp, Alloc> &range, const It &where, Args &&...args)
    {
        auto [it, b] = range.emplace(std::forward<Args>(args)...);
        success = b;
        return it;
    }

    template <typename K, typename T, typename Cmp, typename Alloc, typename It, typename... Args>
    requires std::constructible_from<std::pair<const K, T>, Args...>
    friend auto tag_invoke(emplace_t, bool &success, std::map<K, T, Cmp, Alloc> &range, const It &where, Args &&...args)
    {
        auto [it, b] = range.emplace(std::forward<Args>(args)...);
        success = b;
        return it;
    }

    template <std::ranges::range R, typename It, typename... Args>
    requires tag_invocable<emplace_t, bool &, R &, const It &, Args...>
    auto operator()(R &range, const It &where, Args &&...args) const
        noexcept(is_nothrow_tag_invocable_v<emplace_t, bool &, R &, const It &, Args...>)
            -> tag_invoke_result_t<emplace_t, bool &, R &, const It &, Args...>
    {
        bool success;
        return tag_invoke(*this, success, range, where, std::forward<Args>(args)...);
    }

    template <std::ranges::range R, typename It, typename... Args>
    requires tag_invocable<emplace_t, bool &, R &, const It &, Args...>
    auto operator()(bool &success, R &range, const It &where, Args &&...args) const
        noexcept(is_nothrow_tag_invocable_v<emplace_t, bool &, R &, const It &, Args...>)
            -> tag_invoke_result_t<emplace_t, bool &, R &, const It &, Args...>
    {
        return tag_invoke(*this, success, range, where, std::forward<Args>(args)...);
    }
};

inline constexpr emplace_t emplace;

}