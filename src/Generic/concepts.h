#pragma once

namespace Engine {

template <auto a>
struct auto_holder;

template <typename, template <typename...> typename>
struct is_instance : std::false_type {
};

template <typename... T, template <typename...> typename U>
struct is_instance<U<T...>, U> : std::true_type {
    using argument_types = type_pack<T...>;
};

template <typename T, template <typename...> typename U>
concept InstanceOf = is_instance<T, U>::value;

template <typename, template <auto...> typename>
struct is_instance_auto : std::false_type {
};

template <auto... V, template <auto...> typename U>
struct is_instance_auto<U<V...>, U> : std::true_type {
    using arguments = auto_holder<V...>;
};

template <typename T, template <auto...> typename U>
concept InstanceOfA = is_instance_auto<T, U>::value;

template <typename, template <auto, typename...> typename>
struct is_instance_auto1 : std::false_type {
};

template <auto A, typename... V, template <auto, typename...> typename U>
struct is_instance_auto1<U<A, V...>, U> : std::true_type {
    using value_argument = auto_holder<A>;
    using arguments = type_pack<V...>;
};

template <typename T, template <auto, typename...> typename U>
concept InstanceOfA1 = is_instance_auto1<T, U>::value;

template <typename T>
concept Tuple = InstanceOf<T, std::tuple>;

template <typename T>
concept String = std::is_constructible_v<std::string, const T &> && std::is_constructible_v<T, const std::string &>;

template <typename T>
concept StringViewable = std::is_constructible_v<std::string_view, const T &>;

template <typename T>
concept Enum = std::is_enum_v<T>;

template <typename T>
concept Pointer = std::is_pointer_v<T> || std::is_null_pointer_v<T>;

template <typename T>
concept Function = std::is_function_v<T>;

template <typename T>
concept Unsigned = std::is_unsigned_v<T>;

template <typename T, typename... Ty>
concept OneOf = (std::same_as<T, Ty> || ...);

template <typename T, typename... Ty>
concept NoneOf = !OneOf<T, Ty...>;

template <typename T, typename... Ty>
concept DecayedNoneOf = NoneOf<std::decay_t<T>, Ty...>;

template <typename T>
concept Reference = std::is_reference_v<T> || InstanceOf<T, std::reference_wrapper>;

}