#pragma once

namespace Engine {
template <typename T>
using identity = T;

template <typename V, typename...>
struct last {
    typedef V type;
};

template <typename U, typename V, typename... T>
struct last<U, V, T...> : last<V, T...> {
};

template <typename V, typename...>
struct first {
    typedef V type;
};

template <typename... Args>
using first_t = typename first<Args...>::type;

template <typename...>
using void_t = void;

template <typename T, typename...>
using dependent_t = T;

template <typename T, bool b>
struct dependent_bool : std::bool_constant<b> {
};

template <typename T, typename I, I i>
struct dependent_constant : std::integral_constant<I, i> {
};

template <typename... Ts>
struct overloaded : Ts... {
    using Ts::operator()...;
};
template <typename... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

template <typename T, size_t S>
constexpr size_t array_size(T (&)[S])
{
    return S;
}

template <template <typename> typename F, typename T, bool b>
struct transform_if {
    using type = T;
};

template <template <typename> typename F, typename T>
struct transform_if<F, T, true> {
    using type = F<T>;
};

template <template <typename> typename F, typename T, bool b>
using transform_if_t = typename transform_if<F, T, b>::type;

template <size_t add, typename Sequence>
struct index_range_add;

template <size_t add, size_t... Is>
struct index_range_add<add, std::index_sequence<Is...>> {
    using type = std::index_sequence<(Is + add)...>;
};

template <size_t from, size_t to>
using make_index_range = typename index_range_add<from, std::make_index_sequence<to - from>>::type;

template <typename Base, std::derived_from<Base> Derived>
constexpr size_t inheritance_offset()
{
    return reinterpret_cast<uintptr_t>(static_cast<Base *>(reinterpret_cast<Derived *>(0x1))) - 1;
}

template <typename T>
struct OutRef {

    OutRef() = default;

    OutRef(T &t)
        : mPtr(&t)
    {
    }

    OutRef &operator=(T &t)
    {
        mPtr = &t;
        return *this;
    }

    operator T &()
    {
        return *mPtr;
    }

    T *mPtr = nullptr;
};

struct Void {
};

template <typename R = Void, typename F, typename... Args>
auto invoke_patch_void(F &&f, Args &&...args)
{
    if constexpr (std::same_as<std::invoke_result_t<F, Args...>, void>) {
        std::invoke(std::forward<F>(f), std::forward<Args>(args)...);
        return R {};
    } else {
        return std::invoke(std::forward<F>(f), std::forward<Args>(args)...);
    }
}

template <typename T, typename R = Void>
using patch_void_t = std::conditional_t<std::same_as<T, void>, R, T>;

template <auto a>
struct auto_holder {
    static constexpr auto value = a;
};

template <typename T>
struct type_holder_t {
    using type = T;
};

template <typename T>
const constexpr type_holder_t<T> type_holder = {};

template <auto f, auto g>
concept FSameAs = std::same_as<auto_holder<f>, auto_holder<g>>;

template <bool b, typename T>
using const_if = std::conditional_t<b, const T, T>;

template <typename T>
decltype(auto) forward_ref(std::remove_reference_t<T> &t)
{
    if constexpr (Reference<T>) {
        return std::ref(t);
    } else {
        return std::forward<T>(t);
    }
}

template <typename T>
decltype(auto) forward_ref(std::remove_reference_t<T> &&t)
{
    if constexpr (Reference<T>) {
        return std::ref(t);
    } else {
        return std::forward<T>(t);
    }
}

template <template <typename> typename Inner>
struct Not {
    template <typename T>
    struct type : std::bool_constant<!Inner<T>::value> {
    };
};

}
