#pragma once

namespace Engine {

struct DefaultTag;

template <typename Tag, size_t>
struct TaggedPlaceholder;

template <size_t N>
using Placeholder = TaggedPlaceholder<DefaultTag, N>;

namespace __Generic_impl__ {
    template <typename T, typename Tag, typename...>
    struct replace_tagged;
}

template <typename T>
struct replace {
    template <typename... Args>
    using type = typename __Generic_impl__::replace_tagged<T, DefaultTag, Args...>::type;

    template <typename Tag, typename... Args>
    using tagged = typename __Generic_impl__::replace_tagged<T, Tag, Args...>::type;
};

namespace __Generic_impl__ {
    template <size_t N, typename... Ty>
    struct select;

    template <size_t N, typename Head, typename... Tail>
    struct select<N, Head, Tail...> : select<N - 1, Tail...> {
    };

    template <typename Head, typename... Tail>
    struct select<0, Head, Tail...> {
        using type = Head;
    };

    template <typename T, typename Tag, typename...>
    struct replace_tagged {
        using type = T;
    };

    template <typename T, typename Tag, typename... Args>
    struct replace_tagged<T &, Tag, Args...> {
        using type = typename replace_tagged<T, Tag, Args...>::type &;
    };

    template <typename Tag, size_t N, typename... Args>
    struct replace_tagged<TaggedPlaceholder<Tag, N>, Tag, Args...> {
        using type = typename select<N, Args...>::type;
    };

    template <template <typename...> typename C, typename... Ty, typename Tag, typename... Args>
    struct replace_tagged<C<Ty...>, Tag, Args...> {
        using type = C<typename replace_tagged<Ty, Tag, Args...>::type...>;
    };

    template <template <auto, typename...> typename C, auto V, typename... Ty, typename Tag, typename... Args>
    struct replace_tagged<C<V, Ty...>, Tag, Args...> {
        using type = C<V, typename replace_tagged<Ty, Tag, Args...>::type...>;
    };

    template <template <template <typename...> typename, typename...> typename C, template <typename...> typename V, typename... Ty, typename Tag, typename... Args>
    struct replace_tagged<C<V, Ty...>, Tag, Args...> {
        using type = C<V, typename replace_tagged<Ty, Tag, Args...>::type...>;
    };
}

}