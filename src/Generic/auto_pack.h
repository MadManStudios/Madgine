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
};

template <auto head, auto... tail>
struct auto_pack<head, tail...> {

    using Tail = auto_pack<tail...>;

    template <template <auto...> class Wrapper>
    using instantiate = Wrapper<head, tail...>;

    template <auto... v>
    using append = auto_pack<head, tail..., v...>;

    template <template <auto, typename> typename Op, typename Init>
    using fold = Op<head, typename Tail::template fold<Op, Init>>;
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

}