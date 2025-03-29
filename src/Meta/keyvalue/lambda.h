#pragma once

#include "metatable.h"
#include "valuetype_forward.h"

namespace Engine {

template <typename Functor>
struct LambdaHolder : ProxyScopeBase, Functor {

    LambdaHolder(Functor &&f)
        : Functor(std::forward<Functor>(f))
    {
    }

    LambdaHolder(const LambdaHolder &) = delete;
    LambdaHolder(LambdaHolder &&) = delete;

    ApiFunction getF() const
    {
        return &sFunctionTable;
    }

    virtual ScopePtr proxyScopePtr() override
    {
        return { static_cast<Functor *>(this), &sMetaTable };
    }

private:
    static const MetaTable *sMetaTablePtr;

    template <typename R, typename T, typename... Args>
    static CONSTEVAL std::array<FunctionArgument, sizeof...(Args) + 1> metafunctionArgsMemberHelper()
    {
        return { { { { { ValueTypeEnum::ScopeValue }, &sMetaTablePtr }, "this" }, { toValueTypeDesc<std::remove_const_t<std::remove_reference_t<Args>>>(), {} }... } };
    }

    template <typename R, typename T, typename... Args>
    static CONSTEVAL std::array<FunctionArgument, sizeof...(Args) + 1> metafunctionArgs(R (T::*f)(Args...))
    {
        return metafunctionArgsMemberHelper<R, T, Args...>();
    }

    template <typename R, typename T, typename... Args>
    static CONSTEVAL std::array<FunctionArgument, sizeof...(Args) + 1> metafunctionArgs(R (T::*f)(Args...) const)
    {
        return metafunctionArgsMemberHelper<R, T, Args...>();
    }

    static constexpr const auto sArgs = metafunctionArgs(&Functor::operator());

    template <auto F, typename R, typename T, typename... Args, size_t... I>
    static void unpackMemberHelper(const FunctionTable *table, ArgumentList &results, const ArgumentList &args, std::index_sequence<I...>)
    {
        ScopePtr scope = ValueType_as<ScopePtr>(getArgument(args, 0));
        assert(scope.mType == &sMetaTable);
        T *t = static_cast<T *>(scope.mScope);
        results = { invoke_patch_void<std::monostate>(F, t, ValueType_as<std::remove_cv_t<std::remove_reference_t<Args>>>(getArgument(args, I + 1))...) };
    }

    template <auto F, typename R, typename T, typename... Args>
    static void unpackMemberApiMethod(const FunctionTable *table, ArgumentList &results, const ArgumentList &args)
    {
        unpackMemberHelper<F, R, T, Args...>(table, results, args, std::make_index_sequence<sizeof...(Args)>());
    }

    template <auto F, typename R, typename T, typename... Args>
    static CONSTEVAL typename FunctionTable::FPtr wrapHelper(R (T::*f)(Args...))
    {
        return &unpackMemberApiMethod<F, R, T, Args...>;
    }

    template <auto F, typename R, typename T, typename... Args>
    static CONSTEVAL typename FunctionTable::FPtr wrapHelper(R (T::*f)(Args...) const)
    {
        return &unpackMemberApiMethod<F, R, T, Args...>;
    }

    static const constexpr FunctionTable sFunctionTable {
        wrapHelper<&Functor::operator()>(&Functor::operator()),
        "Lambda",
        CallableTraits<decltype(&Functor::operator())>::argument_count,
        true,
        sArgs.data()
    };

    static void sGetter(ValueType &retVal, const ScopePtr &scope)
    {
        assert(scope.mType == &sMetaTable);
        to_ValueType(retVal, BoundApiFunction { &sFunctionTable, scope });
    }

    static const constexpr std::pair<const char *, Accessor> sMembers[2] {
        { "__call", { &sGetter, nullptr, toValueTypeDesc<BoundApiFunction>() } },
        { nullptr, { nullptr, nullptr, ExtendedValueTypeDesc { ExtendedValueTypeEnum::GenericType } } }
    };

    static const constexpr MetaTable sMetaTable {
        &sMetaTablePtr,
        "<Lambda>",
        sMembers
    };
};

template <typename F>
const MetaTable *LambdaHolder<F>::sMetaTablePtr = &LambdaHolder<F>::sMetaTable;

template <typename F>
OwnedScopePtr lambda(F &&f)
{
    return std::static_pointer_cast<ProxyScopeBase>(std::make_shared<LambdaHolder<F>>(std::forward<F>(f)));
}

}