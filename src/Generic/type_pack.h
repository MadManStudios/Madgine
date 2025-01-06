#pragma once

namespace Engine {

template <typename Sequence, template <size_t...> typename T>
struct index_sequence_instantiate;

template <size_t... Is, template <size_t...> typename T>
struct index_sequence_instantiate<std::index_sequence<Is...>, T> {
    using type = T<Is...>;
};

template <typename Sequence, template <size_t...> typename T>
using index_sequence_instantiate_t = typename index_sequence_instantiate<Sequence, T>::type;

template <typename...>
struct type_pack;

template <>
struct type_pack<> {

    static constexpr const size_t size = 0;

    using indices = std::index_sequence<>;

    template <typename... T>
    using append = type_pack<T...>;
    template <typename... T>
    using prepend = type_pack<T...>;
    template <bool Cond, typename T>
    using prepend_if = std::conditional_t<Cond, type_pack<T>, type_pack<>>;
    template <typename Pack2>
    using concat = Pack2;

    template <template <typename...> typename Filter, typename... Args>
    using filter = type_pack<>;

    template <template <typename...> typename Wrapper>
    using instantiate = Wrapper<>;
    template <template <typename, typename> typename Op, typename Init>
    using fold = Init;

    using as_tuple = instantiate<std::tuple>;

    template <template <typename> typename F>
    using transform = type_pack<>;
    template <template <size_t> typename F, size_t offset = 0>
    using transform_index = type_pack<>;
    template <template <typename, size_t> typename F, size_t offset = 0>
    using transform_with_index = type_pack<>;

    template <typename T>
    using unique = T;

    template <typename Default>
    using unpack_unique = Default;

    template <size_t I>
    using select = std::enable_if_t<dependent_bool<std::integral_constant<size_t, I>, false>::value>;
};

template <typename Head, typename... Ty>
struct type_pack<Head, Ty...> {

    using Tail = type_pack<Ty...>;

    struct helpers {
        template <size_t I>
        struct recurse : Tail::helpers::template recurse<I - 1> {
        };

        template <>
        struct recurse<0> {
            using type = Head;
        };

        template <typename, typename T>
        struct is_or_contains : std::false_type {
            template <typename IntType, typename...>
            static constexpr IntType index = 1 + Tail::template index<IntType, T>;
        };

        template <typename T>
        struct is_or_contains<T, T> : std::true_type {
            template <typename IntType, typename...>
            static constexpr IntType index = 0;
        };

        template <typename... Ty2, typename T>
        requires type_pack<Ty2...>::template contains<T> struct is_or_contains<type_pack<Ty2...>, T> : is_or_contains<T, T> {
        };
    };

    using first = Head;

    using indices = std::index_sequence_for<Head, Ty...>;
    static constexpr const size_t size = 1 + sizeof...(Ty);

    template <template <typename...> typename Filter, typename... Args>
    using filter = typename Tail::template filter<Filter>::template prepend_if<Filter<Head, Args...>::value, Head>;

    using pop_front = Tail;
    template <typename... T>
    using append = type_pack<Head, Ty..., T...>;
    template <typename... T>
    using prepend = type_pack<T..., Head, Ty...>;
    template <bool Cond, typename T>
    using prepend_if = std::conditional_t<Cond, type_pack<T, Head, Ty...>, type_pack<Head, Ty...>>;

    template <typename Pack2>
    using concat = typename Pack2::template prepend<Head, Ty...>;

    template <template <typename> typename F>
    using transform = type_pack<F<Head>, F<Ty>...>;
    template <template <size_t> typename F, size_t offset = 0>
    using transform_index = typename Tail::template transform_index<F, offset + 1>::template prepend<F<offset>>;
    template <template <typename, size_t> typename F, size_t offset = 0>
    using transform_with_index = typename Tail::template transform_with_index<F, offset + 1>::template prepend<F<Head, offset>>;

    template <template <typename...> typename Wrapper>
    using instantiate = Wrapper<Head, Ty...>;
    template <template <typename, typename> typename Op, typename Init>
    using fold = Op<Head, typename Tail::template fold<Op, Init>>;

    using as_tuple = instantiate<std::tuple>;

    template <size_t I>
    using select = typename helpers::template recurse<I>::type;
    template <size_t... Is>
    using select_multiple = type_pack<select<Is>...>;
    template <size_t n>
    using select_first_n = index_sequence_instantiate_t<std::make_index_sequence<n>, select_multiple>;

    template <typename T>
    static constexpr bool contains = (helpers::template is_or_contains<Head, T>::value || ... || helpers::template is_or_contains<Ty, T>::value);

    template <typename IntType, typename T>
    static constexpr IntType index = helpers::template is_or_contains<Head, T>::template index<IntType, Ty...>;

    template <typename T>
    struct unique {
        static_assert(dependent_bool<T, false>::value, "unpack_unique passed to type_pack containing 2 or more elements");
    };

    template <typename Default = void>
    using unpack_unique = typename Tail::template unique<Head>;
};

template <typename Pack>
using type_pack_first = typename Pack::first;

template <typename Pack>
using type_pack_as_tuple = typename Pack::as_tuple;

template <typename Pack1, typename Pack2>
using type_pack_concat = typename Pack1::template concat<Pack2>;

template <typename T>
struct type_pack_appender {
    template <typename Pack>
    using type = typename Pack::template append<T>;
};

namespace __generic_impl__ {
    template <typename T, size_t I>
    using lift_by_index = T;

    template <typename T, typename Is>
    struct type_pack_repeat_n_times_helper;

    template <typename T, size_t... Is>
    struct type_pack_repeat_n_times_helper<T, std::index_sequence<Is...>> {
        using type = type_pack<lift_by_index<T, Is>...>;
    };
}

template <typename T, size_t n>
using type_pack_repeat_n_times = typename __generic_impl__::type_pack_repeat_n_times_helper<T, std::make_index_sequence<n>>::type;

template <typename T>
struct to_type_pack_helper;

template <template <typename...> typename Outer, typename... Ty>
struct to_type_pack_helper<Outer<Ty...>> {
    using type = type_pack<Ty...>;
};

template <typename T>
using to_type_pack = typename to_type_pack_helper<T>::type;

}