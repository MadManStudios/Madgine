#pragma once

namespace Engine {

template <auto...>
struct auto_pack;

template <>
struct auto_pack<> {

	template <template <auto...> class Wrapper>
	using instantiate = Wrapper<>;

};

template <auto head, auto... tail>
struct auto_pack<head, tail...> {

	using Tail = auto_pack<tail...>;

    template <template <auto...> class Wrapper>
    using instantiate = Wrapper<head, tail...>;
};

}