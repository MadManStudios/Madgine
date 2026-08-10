#pragma once

#include "Generic/fixed_string.h"
#include "Generic/guard.h"
#include "Generic/linestruct.h"

#include "functiontable_impl.h"

#include "accessor.h"
#include "boundapifunction.h"
#include "dynamicaccessorlist.h"
#include "metatable.h"
#include "util.h"

namespace Engine {
namespace Reflect {

    template <typename T>
    struct Derived {
    };

    template <typename T>
    struct Variadic {
        using type = T;
    };

    namespace __Reflect_impl__ {

        struct MetaTableTag;
        template <typename T>
        struct MetaMemberFunctionTag;

        template <typename Scope, auto Getter, auto Setter>
        constexpr Accessor property(const char *name)
        {
            using getter_traits = CallableTraits<decltype(Getter)>;
            using T = typename getter_traits::return_type;

            Result (*setter)(const Accessor *, const Value &, const Value &, ContextPtr) = nullptr;

            if constexpr (Setter != nullptr) {
                setter = [](const Accessor *, const Value &scope, const Value &v, ContextPtr context) -> Result {
                    return Reflect::invoke_member(Setter, context, scope, v);
                };
            }

            return {
                name,
                nullptr,
                [](const Accessor *, Value &retVal, const Value &scope, ContextPtr context) -> Result {
                    return Reflect::invoke_member(retVal, Getter, context, scope);
                },
                setter,
                toType<forward_ref_t<T>>()
            };
        }

        template <auto P, typename Scope, typename T>
        void setField(Scope *s, const T &t)
        {
            s->*P = t;
        }

        template <typename Scope, auto P>
        constexpr Accessor member(const char *name)
        {
            using traits = CallableTraits<decltype(P)>;
            using DerivedScope = typename traits::class_type;
            using T = std::remove_reference_t<typename traits::return_type>;

            if constexpr (std::is_const_v<DerivedScope> || !std::is_assignable_v<T &, const T &> || (std::ranges::range<T> && !Concepts::String<T>)) {
                return property<Scope, P, nullptr>(name);
            } else {
                return property<Scope, P, &setField<P, DerivedScope, T>>(name);
            }
        }

        template <auto F, typename T>
        static constexpr TypedBoundApiFunction<&function<F>> method(T &scope)
        {
            return { &scope };
        }

    }
}
}

#define METATABLE_BEGIN(T) \
    METATABLE_BEGIN_BASE(T, void)

#define METATABLE_BEGIN_BASE(T, Base) \
    METATABLE_BEGIN_BASE_EX(, T, Base)

#define METATABLE_BEGIN_EX(Idx, T) \
    METATABLE_BEGIN_BASE_EX(Idx, T, void)

#define METATABLE_BEGIN_BASE_EX(Idx, T, Base)                                                           \
    namespace Engine {                                                                                  \
        namespace __generic_impl__ {                                                                    \
            START_STRUCT(Reflect::__Reflect_impl__::MetaTableTag, Idx)                                  \
            {                                                                                           \
                using BaseT = Base;                                                                     \
                using Ty = T;                                                                           \
                constexpr const Reflect::Accessor *data(const Reflect::Accessor *p) const { return p; } \
                static constexpr const fixed_string name = #T;                                          \
            };                                                                                          \
        }                                                                                               \
    }

#define METATABLE_ENTRY_EX(Idx, Acc)                                                                \
    namespace Engine {                                                                              \
        namespace __generic_impl__ {                                                                \
            LINE_STRUCT(Reflect::__Reflect_impl__::MetaTableTag, Idx)                               \
            {                                                                                       \
                constexpr const Reflect::Accessor *data(const Reflect::Accessor *) const            \
                {                                                                                   \
                    return BASE_STRUCT(Reflect::__Reflect_impl__::MetaTableTag, Idx)::data(&mData); \
                }                                                                                   \
                Reflect::Accessor mData = Acc;                                                      \
            };                                                                                      \
        }                                                                                           \
    }

#define DYNAMIC_ENTRY_EX(Idx, Builder, Init)                                                              \
    namespace Engine {                                                                                    \
        namespace __generic_impl__ {                                                                      \
            LINE_STRUCT(Reflect::__Reflect_impl__::MetaTableTag, Idx)                                     \
            {                                                                                             \
                void init() const                                                                         \
                {                                                                                         \
                    mData.init();                                                                         \
                }                                                                                         \
                                                                                                          \
                constexpr const Reflect::Accessor *data(const Reflect::Accessor *) const                  \
                {                                                                                         \
                    return BASE_STRUCT(Reflect::__Reflect_impl__::MetaTableTag, Idx)::data(mData.data()); \
                }                                                                                         \
                mutable Reflect::DynamicAccessorList<Builder, Init> mData;                                \
            };                                                                                            \
        }                                                                                                 \
    }

#define DYNAMIC_INITIALIZATION(T)     \
    namespace Meta_##T                \
    {                                 \
        static Engine::Guard sInit {  \
            []() { sMembers.init(); } \
        };                            \
    }

#define METATABLE_END(T) \
    METATABLE_END_EX     \
    (, T)

#define METATABLE_DYNAMIC_END(Builder, Init, T) \
    METATABLE_DYNAMIC_END_EX                    \
    (, Builder, Init, T)

#define METATABLE_END_EX(Idx, T)                  \
    METATABLE_ENTRY_EX(Idx, Reflect::Accessor {}) \
    METATABLE_INSTANTIATION(Idx, T)

#define METATABLE_DYNAMIC_END_EX(Idx, Builder, Init, T) \
    DYNAMIC_ENTRY_EX(Idx, Builder, Init)                \
    METATABLE_INSTANTIATION(Idx, T)                     \
    DYNAMIC_INITIALIZATION(T)

#define METATABLE_INSTANTIATION(Idx, T)                                                                                                                                                                                                                               \
    namespace Meta_##T                                                                                                                                                                                                                                                \
    {                                                                                                                                                                                                                                                                 \
        static constexpr GET_STRUCT(::Engine::Reflect::__Reflect_impl__::MetaTableTag, Idx) sMembers = {};                                                                                                                                                            \
    }                                                                                                                                                                                                                                                                 \
    DLL_EXPORT_VARIABLE(constexpr, const ::Engine::Reflect::MetaTable, , table, SINGLE_ARG({ #T, ::Engine::type_holder<T>, ::Engine::type_holder<GET_STRUCT(::Engine::Reflect::__Reflect_impl__::MetaTableTag, Idx)::BaseT>, Meta_##T::sMembers.data(nullptr) }), T); \
    namespace Meta_##T                                                                                                                                                                                                                                                \
    {                                                                                                                                                                                                                                                                 \
        static ::Engine::Reflect::__Reflect_impl__::MetaTableRegistrator<T> __reg;                                                                                                                                                                                    \
    }

#define NAMED_MEMBER_EX(Idx, Name, M) \
    METATABLE_ENTRY_EX(Idx, SINGLE_ARG(::Engine::Reflect::__Reflect_impl__::member<Ty, &Ty::M>(STRINGIFY(Name))))

#define NAMED_MEMBER(Name, M) \
    NAMED_MEMBER_EX(, Name, M)

#define MEMBER_EX(Idx, M) \
    NAMED_MEMBER_EX(Idx, M, M)

#define MEMBER(M) \
    MEMBER_EX(, M)

#define READONLY_PROPERTY_EX(Idx, Name, Getter) \
    METATABLE_ENTRY_EX(Idx, SINGLE_ARG(::Engine::Reflect::__Reflect_impl__::property<Ty, &Ty::Getter, nullptr>(#Name)))

#define READONLY_PROPERTY(Name, Getter) \
    READONLY_PROPERTY_EX(, Name, Getter)

#define PROPERTY_EX(Idx, Name, Getter, Setter) \
    METATABLE_ENTRY_EX(Idx, SINGLE_ARG(::Engine::Reflect::__Reflect_impl__::property<Ty, &Ty::Getter, &Ty::Setter>(#Name)))

#define PROPERTY(Name, Getter, Setter) \
    PROPERTY_EX(, Name, Getter, Setter)

#define NAMED_FUNCTION_EX(Idx, Name, F, ...)                                                                                                                                                                                                                                                                                                          \
    FUNCTIONTABLE_EX(BASE_STRUCT(::Engine::Reflect::__Reflect_impl__::MetaTableTag, Idx)::name + "::" STRINGIFY(Name), ::Engine::Reflect::__Reflect_impl__::MetaMemberFunctionTag<BASE_STRUCT(::Engine::Reflect::__Reflect_impl__::MetaTableTag, Idx)::Ty>, BASE_STRUCT(::Engine::Reflect::__Reflect_impl__::MetaTableTag, Idx)::Ty::F, #__VA_ARGS__) \
    METATABLE_ENTRY_EX(Idx, SINGLE_ARG(::Engine::Reflect::__Reflect_impl__::property<Ty, &::Engine::Reflect::__Reflect_impl__::method<&Ty::F, Ty>, nullptr>(STRINGIFY(Name))))

#define NAMED_FUNCTION(Name, F, ...) \
    NAMED_FUNCTION_EX(, Name, F, __VA_ARGS__)

#define FUNCTION_EX(Idx, F, ...) \
    NAMED_FUNCTION_EX(Idx, F, F, __VA_ARGS__)

#define FUNCTION(F, ...) \
    FUNCTION_EX(, F, __VA_ARGS__)

#define PROXY_EX(Idx, Getter) \
    READONLY_PROPERTY_EX(Idx, __proxy, Getter)

#define PROXY(Getter) \
    PROXY_EX(, Getter)

#define CALL_OPERATOR_EX(Idx, ...) \
    NAMED_FUNCTION_EX(Idx, __call, operator(), __VA_ARGS__)

#define CALL_OPERATOR(...) \
    CALL_OPERATOR_EX(, __VA_ARGS__)
