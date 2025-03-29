#pragma once

#include "typedscopeptr.h"

namespace Engine {

template <typename F>
void kvScan(F &&f, const ScopePtr &scope, std::string_view name = {}, ScopePtr parent = {})
{
    if constexpr (std::is_invocable_v<F, ScopePtr, std::string_view, ScopePtr>) {
        f(scope, name, parent);
    }

    ValueType v;
    for (ScopeField field : scope) {
        field.value(v);
        v.visit([&](auto &&v) { kvScan(f, std::forward<decltype(v)>(v), field.key(), scope); });
    }
} 
    
template <typename F, typename T>
void kvScan(F &&f, const T &t, std::string_view name = {}, ScopePtr parent = {})
{
    if constexpr (std::is_invocable_v<F, T, std::string_view, ScopePtr>) {
        f(t, name, parent);
    }
}

}