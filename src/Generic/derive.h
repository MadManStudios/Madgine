#pragma once

namespace Engine {

namespace __generic_impl__ {
    template <int I>
    struct prio {
    };

    template <int I>
    struct upToPrio : upToPrio<I - 1> {
    };

    template <>
    struct upToPrio<0> {
    };

    struct Dummy {
        template <typename T>
        operator T();
    };
}

#define DERIVE_FRIEND(Name, ...) \
    template <typename...>       \
    friend struct __VA_ARGS__ __derive_impl__##Name##_def;

#define DERIVE_FUNCTION2(Name, Fn, ...)                                                                                                                                                                       \
    template <typename... Args>                                                                                                                                                                               \
    struct __derive_impl__##Name##_def {                                                                                                                                                                      \
        template <typename T>                                                                                                                                                                                 \
        static auto derive(T &&t, Engine::__generic_impl__::prio<0>) -> decltype(std::forward<T>(t).Fn(std::declval<Args>()...));                                                                             \
        template <typename T>                                                                                                                                                                                 \
        static auto derive(T &&t, Engine::__generic_impl__::upToPrio<0>) -> decltype(std::forward<T>(t).Fn(std::declval<Args>()...));                                                                         \
                                                                                                                                                                                                              \
        template <typename T>                                                                                                                                                                                 \
        static auto derive(T &&t, Engine::__generic_impl__::prio<1>) -> decltype(std::forward<T>(t).Fn(std::declval<Args>()..., Engine::__generic_impl__::Dummy {}));                                         \
        template <typename T>                                                                                                                                                                                 \
        static auto derive(T &&t, Engine::__generic_impl__::upToPrio<1>) -> decltype(std::forward<T>(t).Fn(std::declval<Args>()..., Engine::__generic_impl__::Dummy {}));                                     \
                                                                                                                                                                                                              \
        template <typename T>                                                                                                                                                                                 \
        static auto derive(T &&t, Engine::__generic_impl__::prio<2>) -> decltype(std::forward<T>(t).Fn(std::declval<Args>()..., Engine::__generic_impl__::Dummy {}, Engine::__generic_impl__::Dummy {}));     \
        template <typename T>                                                                                                                                                                                 \
        static auto derive(T &&t, Engine::__generic_impl__::upToPrio<2>) -> decltype(std::forward<T>(t).Fn(std::declval<Args>()..., Engine::__generic_impl__::Dummy {}, Engine::__generic_impl__::Dummy {})); \
    };                                                                                                                                                                                                        \
    namespace __generic_impl__ {                                                                                                                                                                              \
        template <typename... Args>                                                                                                                                                                           \
        struct __derive_impl__##Name##_helper {                                                                                                                                                               \
            template <typename... Args2>                                                                                                                                                                      \
            using type = __derive_impl__##Name##_def<Args..., Args2...>;                                                                                                                                      \
        };                                                                                                                                                                                                    \
        template <typename... Args>                                                                                                                                                                           \
        using __derive_impl__##Name = typename __derive_impl__##Name##_helper<__VA_ARGS__>::type<Args...>;                                                                                                    \
                                                                                                                                                                                                              \
        template <typename T, typename, typename prio, typename... Args>                                                                                                                                      \
        struct has_function_##Name##_select : std::false_type {                                                                                                                                               \
            using return_type = void;                                                                                                                                                                         \
        };                                                                                                                                                                                                    \
                                                                                                                                                                                                              \
        template <typename T, typename prio, typename... Args>                                                                                                                                                \
        struct has_function_##Name##_select<T, std::void_t<decltype(__derive_impl__##Name<Args...>::derive(std::declval<T>(), prio {}))>, prio, Args...> : std::true_type {                                   \
            using return_type = decltype(__derive_impl__##Name<Args...>::derive(std::declval<T>(), prio {}));                                                                                                 \
        };                                                                                                                                                                                                    \
    }                                                                                                                                                                                                         \
                                                                                                                                                                                                              \
    template <typename T, typename prio = Engine::__generic_impl__::prio<0>, typename... Args>                                                                                                                \
    using has_function_##Name = __generic_impl__::has_function_##Name##_select<T, void, prio, Args...>;                                                                                                       \
    template <typename T, typename... Args>                                                                                                                                                                   \
    constexpr bool has_function_##Name##_v = has_function_##Name<T, Engine::__generic_impl__::prio<0>, Args...>::value;

#define DERIVE_FUNCTION(Name, ...) DERIVE_FUNCTION2(Name, Name, __VA_ARGS__)

/* #define DERIVE_FRIEND(Name, ...) \
    template <typename...>       \
    friend struct __VA_ARGS__ __generic_impl__::derive_##Name##_helper;

#define DERIVE_FUNCTION(Name, ...)                                                                                                                                                               \
    namespace __generic_impl__ {                                                                                                                                                                 \
        template <typename... Args>                                                                                                                                                              \
        struct derive_##Name##_helper {                                                                                                                                                          \
            template <typename T, typename... Args2>                                                                                                                                             \
            static constexpr bool value = requires(T & t)                                                                                                                                        \
            {                                                                                                                                                                                    \
                &T::Name;                                                                                                                                                                        \
                t.Name(std::declval<Args>()..., std::declval<Args2>()...);                                                                                                                       \
            };                                                                                                                                                                                   \
        };                                                                                                                                                                                       \
        template <typename V, typename T, typename... Args>                                                                                                                                      \
        struct derive_##Name##_struct : V {                                                                                                                                                      \
            using return_type = void;                                                                                                                                                            \
        };                                                                                                                                                                                       \
        template <typename T, typename... Args>                                                                                                                                                  \
        struct derive_##Name##_struct<std::true_type, T, Args...> : std::true_type {                                                                                                             \
            using return_type = decltype(std::declval<T>().Name(std::declval<Args>()...));                                                                                                       \
        };                                                                                                                                                                                       \
    }                                                                                                                                                                                            \
    template <typename T, typename... Args>                                                                                                                                                      \
    using has_function_##Name = __generic_impl__::derive_##Name##_struct<std::bool_constant<__generic_impl__::derive_##Name##_helper<__VA_ARGS__>::value<T, Args...>>, T, __VA_ARGS__, Args...>; \
    template <typename T, typename... Args>                                                                                                                                                      \
    constexpr bool has_function_##Name##_v = has_function_##Name<T, Args...>::value;
    */
#define DERIVE_OPERATOR(Name, Operator)                    \
    template <typename T, typename Arg>                    \
    concept has_operator_##Name = requires(T t, Arg arg) { \
        t Operator arg;                                    \
    };

#define DERIVE_TYPENAME(Name)                \
    template <typename T>                    \
    concept has_typename_##Name = requires { \
        typename T::Name;                    \
    };
}