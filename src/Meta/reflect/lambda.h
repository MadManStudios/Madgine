#pragma once

#include "metatable.h"
#include "util.h"

namespace Engine {
namespace Reflect {
    /*
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
            return { { { { { Reflect::TypeEnum::ScopeValue }, &sMetaTablePtr }, "this" }, { Reflect::toType<std::remove_const_t<std::remove_reference_t<Args>>>(), {} }... } };
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
        static Reflect::Result unpackMemberHelper(const FunctionTable *table, Value &retVal, const ArgumentList &args, std::index_sequence<I...>)
        {
            return ValueType_unwrap(retVal, F, getArgument(args, 0), getArgument(args, I + 1)...);
        }

        template <auto F, typename R, typename T, typename... Args>
        static Reflect::Result unpackMemberApiMethod(const FunctionTable *table, ArgumentList &results, const ArgumentList &args)
        {
            return unpackMemberHelper<F, R, T, Args...>(table, results, args, std::make_index_sequence<sizeof...(Args)>());
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

        static Reflect::Result sGetter(const Accessor *, Value &retVal, const Value &scope)
        {
            return ValueType_unwrap(retVal, [](ScopePtr scope) { return BoundApiFunction { &sFunctionTable, scope }; }, scope);
        }

        static const constexpr Accessor sMembers[2] {
            { "__call", nullptr, &sGetter, nullptr, Reflect::toType<BoundApiFunction>() },
            {}
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
    */
}
}