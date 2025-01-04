#pragma once

namespace Engine {

template <typename F, typename R, typename T, typename... _Ty>
struct CallableType {

    typedef F type;
    typedef R return_type;
    typedef T class_type;
    typedef type_pack<_Ty...> argument_types;
    typedef type_pack<std::decay_t<_Ty>...> decay_argument_types;

    static constexpr const bool is_member_function = !std::same_as<T, void>;

    static constexpr size_t argument_count = sizeof...(_Ty) + (is_member_function ? 1 : 0);

    template <template <typename, typename, typename...> typename C, typename... Args>
    using instance = C<Args..., R, T, _Ty...>;    
};

namespace __generic_impl__ {

    template <typename T>
    struct StaticCallableMaker;

    template <typename F, typename R, typename T, typename... _Ty>
    struct StaticCallableMaker<CallableType<F, R, T, _Ty...>> {
        typedef CallableType<T, R, void, _Ty...> type;
    };

    template <typename T>
    using makeStaticCallable = typename StaticCallableMaker<T>::type;

    template <typename R, typename T, typename... _Ty>
    CallableType<R (T::*)(_Ty...), R, T, _Ty...> callableTypeDeducer(R (T::*f)(_Ty...));

    template <typename R, typename T, typename... _Ty>
    CallableType<R (T::*)(_Ty...) const, R, const T, _Ty...> callableTypeDeducer(R (T::*f)(_Ty...) const);

    template <typename R, typename T>
    CallableType<R(T::*), R &, T> callableTypeDeducer(R(T::*p));

    template <typename R, typename... _Ty>
    CallableType<R (*)(_Ty...), R, void, _Ty...> callableTypeDeducer(R (*f)(_Ty...));

    template <typename F, typename R = makeStaticCallable<decltype(callableTypeDeducer(&F::operator()))>>
    R callableTypeDeducer(const F &);

    CallableType<std::nullptr_t, void, void, void> callableTypeDeducer(std::nullptr_t);
}

template <typename F>
using CallableTraits = decltype(__generic_impl__::callableTypeDeducer(std::declval<F>()));

namespace __generic_impl__ {
    template <typename F, typename = void>
    struct callable_is_variadic : std::true_type {
    };

    template <typename F>
    struct callable_is_variadic<F, std::void_t<CallableTraits<F>>> : std::false_type {
    };

    template <typename F>
    constexpr const bool callable_is_variadic_v = callable_is_variadic<F>::value;
}

template <typename F>
constexpr size_t callable_argument_count(size_t expected)
{
    if constexpr (__generic_impl__::callable_is_variadic_v<F>) {
        return expected;
    } else {
        return CallableTraits<F>::argument_count;
    }
}

template <typename F>
constexpr size_t callable_argument_count()
{
    return CallableTraits<F>::argument_count;
}

template <auto f>
constexpr size_t callable_argument_count()
{
    return CallableTraits<decltype(f)>::argument_count;
}

namespace __generic_impl__ {
    template <template <auto, typename, typename, typename...> typename C, auto f>
    struct Partial {
        template <typename Arg1, typename Arg2, typename... Args>
        struct apply {
            using type = C<f, Arg1, Arg2, Args...>;
        };
    };
}

template <auto f>
struct Callable {
    using traits = CallableTraits<decltype(f)>;
    //static constexpr typename traits::type value = f;

    template <template <auto, typename, typename, typename...> typename C, typename... Args>
    struct instance {
        using type = typename traits::template instance<__generic_impl__::Partial<C, f>::template apply, Args...>::type;
    };
};

template <template <auto, typename, typename, typename...> typename C, auto f, typename... Args>
using FunctionCapture = typename Callable<f>::template instance<C, Args...>::type;
}