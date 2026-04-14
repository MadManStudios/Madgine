#pragma once

#include "Generic/linestruct.h"

#include "functionargument.h"
#include "functiontable.h"
#include "valuetype_forward.h"

namespace Engine {

struct MetaFunctionTag;

template <size_t I>
struct MetaFunction {
    static constexpr size_t argumentCount = I;

    std::array<FunctionArgument, I> mArguments;
    ExtendedValueTypeDesc mReturn;
};

template <typename R, typename... Args, size_t... Is>
static constexpr MetaFunction<sizeof...(Args)> metafunctionHelper(std::string_view args, std::index_sequence<Is...>)
{
    std::array<std::string_view, sizeof...(Args)> argumentNames = StringUtil::tokenize<sizeof...(Args)>(args, ',');
    return { { { { toValueTypeDesc<std::remove_const_t<std::remove_reference_t<Args>>>(), argumentNames[Is] }... } }, ::Engine::toValueTypeDesc<::Engine::patch_void_t<R, std::monostate>>() };
}

template <typename R, typename T, typename... Args, size_t... Is>
static constexpr MetaFunction<sizeof...(Args) + 1> metafunctionMemberHelper(std::string_view args, std::index_sequence<Is...>)
{
    std::array<std::string_view, sizeof...(Args)> argumentNames = StringUtil::tokenize<sizeof...(Args)>(args, ',');
    return { { { { toValueTypeDesc<T *>(), "this" }, { toValueTypeDesc<std::remove_const_t<std::remove_reference_t<Args>>>(), argumentNames[Is] }... } }, ::Engine::toValueTypeDesc<::Engine::patch_void_t<R, std::monostate>>() };
}

template <typename R, typename... Args>
static constexpr auto metafunction(R (*f)(Args...), std::string_view args)
{
    return metafunctionHelper<R, Args...>(args, std::index_sequence_for<Args...>());
}

template <typename R, typename T, typename... Args>
static constexpr auto metafunction(R (T::*f)(Args...), std::string_view args)
{
    return metafunctionMemberHelper<R, T, Args...>(args, std::index_sequence_for<Args...>());
}

template <typename R, typename T, typename... Args>
static constexpr auto metafunction(R (T::*f)(Args...) const, std::string_view args)
{
    return metafunctionMemberHelper<R, T, Args...>(args, std::index_sequence_for<Args...>());
}

template <typename T, typename... Args>
static constexpr auto metafunction(void (T::*f)(ArgumentList &, Args...), std::string_view args)
{
    return metafunctionMemberHelper<ValueType, T, Args...>(args, std::index_sequence_for<Args...>());
}

template <typename T, typename... Args>
static constexpr auto metafunction(void (T::*f)(ValueType &, Args...), std::string_view args)
{
    return metafunctionMemberHelper<ValueType, T, Args...>(args, std::index_sequence_for<Args...>());
}

template <typename T, typename... Args>
static constexpr auto metafunction(KeyValueResult (T::*f)(ArgumentList &, Args...), std::string_view args)
{
    return metafunctionMemberHelper<ValueType, T, Args...>(args, std::index_sequence_for<Args...>());
}

template <typename T, typename... Args>
static constexpr auto metafunction(KeyValueResult (T::*f)(ValueType &, Args...), std::string_view args)
{
    return metafunctionMemberHelper<ValueType, T, Args...>(args, std::index_sequence_for<Args...>());
}

template <auto F, size_t... Is>
static constexpr typename FunctionTable::FPtr wrapHelper(std::index_sequence<Is...>)
{
    return [](const FunctionTable *, ValueType &retVal, const ArgumentList &args) {
        return ValueType_unwrap(retVal, F, getArgument(args, Is)...);
    };
}
}

#define FUNCTIONTABLE_IMPL(F, NameInit, Name, Tag, ArgNames)                                                                                                                                                                                                                                                                                                             \
    namespace Engine {                                                                                                                                                                                                                                                                                                                                                   \
        template <>                                                                                                                                                                                                                                                                                                                                                      \
        struct LineStruct<Tag, __LINE__> {                                                                                                                                                                                                                                                                                                                               \
            static constexpr const auto meta = metafunction(&F, ArgNames);                                                                                                                                                                                                                                                                                               \
            NameInit                                                                                                                                                                                                                                                                                                                                                     \
        };                                                                                                                                                                                                                                                                                                                                                               \
    }                                                                                                                                                                                                                                                                                                                                                                    \
    DLL_EXPORT_VARIABLE(constexpr, const ::Engine::FunctionTable, , function, SINGLE_ARG({ ::Engine::wrapHelper<&F>(std::make_index_sequence<::Engine::LineStruct<Tag, __LINE__>::meta.argumentCount> {}), Name, ::Engine::CallableTraits<decltype(&F)>::argument_count, ::Engine::CallableTraits<decltype(&F)>::is_member_function, ::Engine::LineStruct<Tag, __LINE__>::meta.mArguments.data(), ::Engine::LineStruct<Tag, __LINE__>::meta.mReturn }), &F); \
    namespace Engine {                                                                                                                                                                                                                                                                                                                                                   \
        static ::Engine::FunctionTableRegistrator<&F> CONCAT2(__reg_, __LINE__);                                                                                                                                                                                                                                                                                         \
    }

#define FUNCTIONTABLE(F, ...) FUNCTIONTABLE_IMPL(F, , #F, ::Engine::MetaFunctionTag, #__VA_ARGS__)
#define FUNCTIONTABLE_EX(Name, Tag, F, ArgNames) FUNCTIONTABLE_IMPL(SINGLE_ARG(F), static constexpr fixed_string name = SINGLE_ARG(Name);, SINGLE_ARG(::Engine::LineStruct<Tag, __LINE__>::name), SINGLE_ARG(Tag), ArgNames)