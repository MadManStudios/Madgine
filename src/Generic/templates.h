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

template <typename T>
struct CaptureHelper {
    template <typename... Args>
    decltype(auto) operator()(Args &&...args) const
    {
        return mT(std::forward<Args>(args)...);
    }
    operator T &()
    {
        return mT;
    }
    T mT;
};

template <typename T>
CaptureHelper<T> forward_capture(T &&t)
{
    return { std::forward<T>(t) };
}

template <typename T>
CaptureHelper<T> forward_capture(T &t)
{
    return { std::forward<T>(t) };
}

template <typename F, typename R>
auto patch_void(F &&f, R &&result)
{
    return [f { forward_capture<F>(f) }, result { forward_capture<R>(result) }]<typename... Args>(Args &&...args) mutable {
        if constexpr (std::same_as<std::invoke_result_t<F, Args...>, void>) {
            std::invoke(std::forward<F>(f), std::forward<Args>(args)...);
            return static_cast<R>(result);
        } else {
            return std::invoke(std::forward<F>(f), std::forward<Args>(args)...);
        }
    };
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
using forward_ref_t = std::conditional_t<
    std::is_lvalue_reference_v<T>,
    std::reference_wrapper<std::remove_reference_t<T>>,
    T>;

template <typename T>
struct decay_ref {
    using type = T;
};

template <InstanceOf<std::reference_wrapper> T>
struct decay_ref<T> {
    using type = T::type &;
};

template <typename T>
using decay_ref_t = decay_ref<T>::type;

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

template <typename T>
struct declval_helper {
    static T value;
};

}
