#pragma once

#include <type_traits>
#include <utility>
#include "callable_traits.h"
#include "auto_pack.h"

namespace _tag_invoke {
void tag_invoke();

struct _fn {
    template <typename CPO, typename... Args>
    constexpr auto operator()(CPO cpo, Args &&...args) const
        noexcept(noexcept(tag_invoke((CPO &&) cpo, (Args &&) args...)))
            -> decltype(tag_invoke((CPO &&) cpo, (Args &&) args...))
    {
        return tag_invoke((CPO &&) cpo, (Args &&) args...);
    }
};

template <typename CPO, typename... Args>
using tag_invoke_result_t = decltype(tag_invoke(std::declval<CPO>(), std::declval<Args>()...));

using yes_type = char;
using no_type = char (&)[2];

template <typename CPO, typename... Args>
auto try_tag_invoke(int) //
    noexcept(noexcept(tag_invoke(
        std::declval<CPO>(), std::declval<Args>()...)))
        -> decltype(static_cast<void>(tag_invoke(
                        std::declval<CPO>(), std::declval<Args>()...)),
            yes_type {});

template <typename CPO, typename... Args>
no_type try_tag_invoke(...) noexcept(false);

template <template <typename...> class T, typename... Args>
struct defer {
    using type = T<Args...>;
};

struct empty {
};
} // namespace _tag_invoke

namespace _tag_invoke_cpo {
inline constexpr _tag_invoke::_fn tag_invoke {};
}
using namespace _tag_invoke_cpo;

template <auto &CPO>
using tag_t = std::remove_cvref_t<decltype(CPO)>;

using _tag_invoke::tag_invoke_result_t;

template <typename CPO, typename... Args>
inline constexpr bool is_tag_invocable_v = (sizeof(_tag_invoke::try_tag_invoke<CPO, Args...>(0)) == sizeof(_tag_invoke::yes_type));

template <typename CPO, typename... Args>
struct tag_invoke_result
    : std::conditional_t<
          is_tag_invocable_v<CPO, Args...>,
          _tag_invoke::defer<tag_invoke_result_t, CPO, Args...>,
          _tag_invoke::empty> {
};

template <typename CPO, typename... Args>
using is_tag_invocable = std::bool_constant<is_tag_invocable_v<CPO, Args...>>;

template <typename CPO, typename... Args>
inline constexpr bool is_nothrow_tag_invocable_v = noexcept(_tag_invoke::try_tag_invoke<CPO, Args...>(0));

template <typename CPO, typename... Args>
using is_nothrow_tag_invocable = std::bool_constant<is_nothrow_tag_invocable_v<CPO, Args...>>;

template <typename CPO, typename... Args>
concept tag_invocable = (sizeof(_tag_invoke::try_tag_invoke<CPO, Args...>(0)) == sizeof(_tag_invoke::yes_type));

template <typename CPO_holder, typename Base, typename R, std::same_as<void> T, typename... V>
struct VirtualCPOBaseHelper : Base {
    using Base::Base;

    using CPO = decltype(CPO_holder::value);

    using mapped_cpos = typename Base::mapped_cpos::template append<CPO_holder::value>;

    virtual R v_tag_invoke(CPO _cpo, V... v) = 0;

    template <typename... Args>
    friend auto tag_invoke(CPO _cpo, VirtualCPOBaseHelper &base, Args &&...args)
    {
        return base.v_tag_invoke(_cpo, std::forward<Args>(args)...);
    }
};

template <auto cpo, typename Base>
using VirtualCPOBase = typename Engine::CallableTraits<typename decltype(cpo)::signature>::template instance<VirtualCPOBaseHelper, Engine::auto_holder<cpo>, Base>;


template <typename CPO, typename Proj, typename Base, typename R, std::same_as<void> T, typename... V>
struct VirtualCPOImplHelper : Base {
    using Base::Base;

    virtual R v_tag_invoke(CPO _cpo, V... v) override {
        return _cpo(Proj::value(*this), std::forward<V>(v)...);
    }
};

template <auto cpo, auto proj, typename Base>
using VirtualCPOImpl = typename Engine::CallableTraits<typename decltype(cpo)::signature>::template instance<VirtualCPOImplHelper, decltype(cpo), Engine::auto_holder<proj>, Base>;

template <auto proj>
struct VirtualCPOsImplHelper {
    template<auto cpo, typename Base>
    using type = VirtualCPOImpl<cpo, proj, Base>;
};

template <typename Base, auto... cpos>
using VirtualCPOsBase = typename Engine::auto_pack<cpos...>::template fold<VirtualCPOBase, Base>;

template <auto proj, typename Base>
using VirtualCPOsImpl = typename Base::mapped_cpos::template fold<VirtualCPOsImplHelper<proj>::template type, Base>;