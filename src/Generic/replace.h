#pragma once

namespace Engine {

struct DefaultTag;

template <typename Tag, size_t>
struct TaggedPlaceholder;

template <size_t N>
using Placeholder = TaggedPlaceholder<DefaultTag, N>;

template <typename T, typename Tag, typename...>
struct replace_tagged;

template <typename T, template <auto...> typename Pattern, template <auto...> typename...>
struct replace_pattern_tagged;

template <typename T>
struct replace {
    template <typename... Args>
    using type = typename replace_tagged<T, DefaultTag, Args...>::type;
    
    template <typename Tag, typename... Args>
    using tagged = typename replace_tagged<T, Tag, Args...>::type;

    template <template <auto...> typename Pattern, template <auto...> typename... Args>
    using pattern_tagged = typename replace_pattern_tagged<T, Pattern, Args...>::type;
};

template <size_t N, typename... Ty>
struct select;

template <size_t N, typename Head, typename... Tail>
struct select<N, Head, Tail...> : select<N - 1, Tail...> {
};

template <typename Head, typename... Tail>
struct select<0, Head, Tail...> {
    using type = Head;
};

template <size_t N, template <auto...> typename... Ty>
struct select_pattern;

template <size_t N, template <auto...> typename Head, template <auto...> typename... Tail>
struct select_pattern<N, Head, Tail...> : select_pattern<N - 1, Tail...> {
};

template <template <auto...> typename Head, template <auto...> typename... Tail>
struct select_pattern<0, Head, Tail...> {
    template <auto... Params>
    using type = Head<Params...>;
};

template <typename T, typename Tag, typename...>
struct replace_tagged {
    using type = T;
};

template <typename T, typename Tag, typename... Args>
struct replace_tagged<T &, Tag, Args...> {
    using type = typename replace_tagged<T, Tag, Args...>::type&;
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

template <typename T, template <auto...> typename Pattern, template <auto...> typename...>
struct replace_pattern_tagged {
    using type = T;
};

template <typename T, template <auto...> typename Pattern, template <auto...> typename... Args>
struct replace_pattern_tagged<T &, Pattern, Args...> {
    using type = typename replace_pattern_tagged<T, Pattern, Args...>::type&;
};

template <auto... Params, size_t N, template <auto...> typename Pattern, template <auto...> typename... Args>
struct replace_pattern_tagged<TaggedPlaceholder<Pattern<Params...>, N>, Pattern, Args...> {
    using type = typename select_pattern<N, Args...>::template type<Params...>;
};

template <template <typename...> typename C, typename... Ty, template <auto...> typename Pattern, template <auto...> typename... Args>
struct replace_pattern_tagged<C<Ty...>, Pattern, Args...> {
    using type = C<typename replace_pattern_tagged<Ty, Pattern, Args...>::type...>;
};

template <template <auto, typename...> typename C, auto V, typename... Ty, template <auto...> typename Pattern, template <auto...> typename... Args>
struct replace_pattern_tagged<C<V, Ty...>, Pattern, Args...> {
    using type = C<V, typename replace_pattern_tagged<Ty, Pattern, Args...>::type...>;
};

template <template <template <typename...> typename, typename...> typename C, template <typename...> typename V, typename... Ty, template <auto...> typename Pattern, template <auto...> typename... Args>
struct replace_pattern_tagged<C<V, Ty...>, Pattern, Args...> {
    using type = C<V, typename replace_pattern_tagged<Ty, Pattern, Args...>::type...>;
};

}