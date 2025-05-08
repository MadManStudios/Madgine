#pragma once

namespace Engine {

template <auto...>
struct auto_pack;

template <>
struct auto_pack<> {

    template <template <auto...> class Wrapper>
    using instantiate = Wrapper<>;

    template <auto... v>
    using append = auto_pack<v...>;

    template <template <auto, typename> typename Op, typename Init>
    using fold = Init;

    template <size_t I>
    static constexpr auto_pack<I> get {};
};

template <auto head, auto... tail>
struct auto_pack<head, tail...> {

    using Tail = auto_pack<tail...>;

    struct helpers {
        template <size_t I>
        struct recurse : Tail::helpers::template recurse<I - 1> {
        };

        template <size_t I> // Workaround for GCC. Replace with template <> struct recurse<0> once fixed
            requires(I == 0)
        struct recurse<I> {
            static constexpr auto value = head;
        };
    };

    template <template <auto...> class Wrapper>
    using instantiate = Wrapper<head, tail...>;

    template <auto... v>
    using append = auto_pack<head, tail..., v...>;

    template <template <auto, typename> typename Op, typename Init>
    using fold = Op<head, typename Tail::template fold<Op, Init>>;

    template <size_t I>
    static constexpr auto get = helpers::template recurse<I>::value;
};

template <std::size_t N>
struct make_index_pack_helper {
    using type = typename make_index_pack_helper<N - 1>::type::template append<N - 1>;
};

template <>
struct make_index_pack_helper<0> {
    using type = auto_pack<>;
};

template <std::size_t N>
using make_index_pack = typename make_index_pack_helper<N>::type;

template <typename... Ty>
using index_pack_for = make_index_pack<sizeof...(Ty)>;

}