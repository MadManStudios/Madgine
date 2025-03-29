#pragma once

#include "Generic/fixed_string.h"
#include "Generic/linestruct.h"
#include "accessor.h"
#include "boundapifunction.h"
#include "functiontable_impl.h"
#include "metatable.h"
#include "valuetype_forward.h"

namespace Engine {

struct MetaTableTag;
struct MetaTableCtorTag;
template <typename T>
struct MetaMemberFunctionTag;

template <size_t... Ids>
using MetaTableLineStruct = LineStruct<MetaTableTag, Ids...>;

template <size_t... Ids>
using MetaTableCtorLineStruct = LineStruct<MetaTableCtorTag, Ids...>;

template <typename Scope, auto Getter, auto Setter>
constexpr Accessor property()
{
    using getter_traits = CallableTraits<decltype(Getter)>;
    using GetterScope = typename getter_traits::class_type;
    using T = typename getter_traits::return_type;

    void (*setter)(const ScopePtr &, const ValueType &) = nullptr;

    if constexpr (Setter != nullptr) {
        using setter_traits = CallableTraits<decltype(Setter)>;
        using SetterScope = typename setter_traits::class_type;

        //TODO remove const in tuple types
        //static_assert(std::is_same_v<typename setter_traits::argument_types, std::tuple<T>>);

        setter = [](const ScopePtr &scope, const ValueType &v) {
            if constexpr (std::same_as<SetterScope, void>) {
                using SetterScope = std::remove_pointer_t<typename setter_traits::argument_types::template select<0>>;
                if constexpr (std::is_convertible_v<Scope &, SetterScope &>) {
                    TupleUnpacker::invoke(Setter, scope_cast<Scope>(scope), ValueType_as<std::decay_t<T>>(v));
                } else {
                    TupleUnpacker::invoke(Setter, scope, ValueType_as<std::decay_t<T>>(v));
                }
            } else {
                static_assert(std::is_convertible_v<Scope &, SetterScope &>);
                TupleUnpacker::invoke(Setter, scope_cast<Scope>(scope), ValueType_as<std::decay_t<T>>(v));
            }
        };
    }

    return {
        [](ValueType &retVal, const ScopePtr &scope) {
            T value = [=]() -> T {
                if constexpr (std::same_as<GetterScope, void>) {
                    using GetterScope = std::remove_pointer_t<typename getter_traits::argument_types::template select<0>>;
                    if constexpr (std::is_convertible_v<Scope &, GetterScope &>) {
                        return TupleUnpacker::invoke(Getter, scope_cast<Scope>(scope));
                    } else {
                        return TupleUnpacker::invoke(Getter, scope);
                    }
                } else {
                    static_assert(std::is_convertible_v<Scope &, GetterScope &>);
                    return TupleUnpacker::invoke(Getter, scope_cast<Scope>(scope));
                }
            }();

            to_ValueType(retVal, forward_ref<T>(value));
        },
        setter,
        toValueTypeDesc<std::decay_t<T>>()
    };
}

template <auto P, typename Scope, typename T>
void setField(Scope *s, const T &t)
{
    s->*P = t;
}

template <typename Scope, auto P>
constexpr Accessor member()
{
    using traits = CallableTraits<decltype(P)>;
    using DerivedScope = typename traits::class_type;
    using T = std::remove_reference_t<typename traits::return_type>;

    if constexpr (std::is_const_v<DerivedScope> || !std::is_assignable_v<T &, const T &> || (std::ranges::range<T> && !String<T>)) {
        return property<Scope, P, nullptr>();
    } else {
        return property<Scope, P, &setField<P, DerivedScope, T>>();
    }
}

template <auto F>
static constexpr TypedBoundApiFunction<&function<F>> method(ScopePtr scope)
{
    return { scope };
}

template <typename T>
struct ctorHelper {
    template <typename... Args>
    static constexpr Constructor ctor()
    {
        if constexpr (std::is_same_v<type_pack<Args...>, type_pack<void>>)
            return nullptr;
        else {
            return []() {
                return OwnedScopePtr {
                    std::make_shared<ScopeWrapper<T>>()
                };
            };
        }
    }
};

}

#define METATABLE_BEGIN(T) \
    METATABLE_BEGIN_BASE(T, void)

#define METATABLE_BEGIN_BASE(T, Base) \
    METATABLE_BEGIN_BASE_EX(, T, Base)

#define METATABLE_BEGIN_EX(Idx, T) \
    METATABLE_BEGIN_BASE_EX(Idx, T, void)

#define METATABLE_BEGIN_BASE_EX(Idx, T, Base)                                \
    namespace Engine {                                                       \
        START_STRUCT(MetaTableTag, Idx)                                      \
        {                                                                    \
            using BaseT = Base;                                              \
            using Ty = T;                                                    \
            static constexpr const bool base = true;                         \
            constexpr const std::pair<const char *, Accessor> *data() const; \
            static constexpr const fixed_string name = #T;                   \
        };                                                                   \
        START_STRUCT(MetaTableCtorTag, Idx)                                  \
        {                                                                    \
            using Ty = T;                                                    \
            constexpr const Constructor *data() const;                       \
            static constexpr const bool base = true;                         \
        };                                                                   \
    }

#define METATABLE_ENTRY_EX(Idx, Name, Acc)                                  \
    namespace Engine {                                                      \
        LINE_STRUCT(MetaTableTag, Idx)                                      \
        {                                                                   \
            constexpr const std::pair<const char *, Accessor> *data() const \
            {                                                               \
                if constexpr (BASE_STRUCT(MetaTableTag, Idx)::base)         \
                    return &mData;                                          \
                else                                                        \
                    return BASE_STRUCT(MetaTableTag, Idx)::data();          \
            }                                                               \
            static constexpr const bool base = false;                       \
            std::pair<const char *, Accessor> mData = { Name, Acc };        \
        };                                                                  \
    }

#define CONSTRUCTOR_EX(Idx, ...)                                        \
    namespace Engine {                                                  \
        LINE_STRUCT(MetaTableCtorTag, Idx)                              \
        {                                                               \
            constexpr const Constructor *data() const                   \
            {                                                           \
                if constexpr (BASE_STRUCT(MetaTableCtorTag, Idx)::base) \
                    return &mData;                                      \
                else                                                    \
                    return BASE_STRUCT(MetaTableCtorTag, Idx)::data();  \
            }                                                           \
            static constexpr const bool base = false;                   \
            Constructor mData = ctorHelper<Ty>::ctor<__VA_ARGS__>();    \
        };                                                              \
    }

#define CONSTRUCTOR(...) CONSTRUCTOR_EX(, __VA_ARGS__)

#define METATABLE_END(T) \
    METATABLE_END_EX(, T)

#define METATABLE_END_EX(Idx, T)                                                                                                                                                                                                                    \
    METATABLE_ENTRY_EX(Idx, nullptr, SINGLE_ARG(Accessor { nullptr, nullptr, ExtendedValueTypeDesc { ExtendedValueTypeEnum::GenericType } }))                                                                                                       \
    CONSTRUCTOR_EX(Idx, void)                                                                                                                                                                                                                       \
    namespace Meta_##T                                                                                                                                                                                                                              \
    {                                                                                                                                                                                                                                               \
        static constexpr GET_STRUCT(::Engine::MetaTableTag, Idx) sMembers = {};                                                                                                                                                                     \
        static constexpr GET_STRUCT(::Engine::MetaTableCtorTag, Idx) sCtors = {};                                                                                                                                                                   \
    }                                                                                                                                                                                                                                               \
    DLL_EXPORT_VARIABLE(constexpr, const ::Engine::MetaTable, , table, SINGLE_ARG({ #T, ::Engine::type_holder<T>, ::Engine::type_holder<GET_STRUCT(::Engine::MetaTableTag, Idx)::BaseT>, Meta_##T::sMembers.data(), Meta_##T::sCtors.data() }), T); \
    namespace Meta_##T                                                                                                                                                                                                                              \
    {                                                                                                                                                                                                                                               \
        static ::Engine::MetaTableRegistrator<T> __reg;                                                                                                                                                                                             \
    }

#define NAMED_MEMBER_EX(Idx, Name, M) \
    METATABLE_ENTRY_EX(Idx, STRINGIFY(Name), SINGLE_ARG(::Engine::member<Ty, &Ty::M>()))

#define NAMED_MEMBER(Name, M) \
    NAMED_MEMBER_EX(, Name, M)

#define MEMBER_EX(Idx, M) \
    NAMED_MEMBER_EX(Idx, M, M)

#define MEMBER(M) \
    MEMBER_EX(, M)

#define READONLY_PROPERTY_EX(Idx, Name, Getter) \
    METATABLE_ENTRY_EX(Idx, #Name, SINGLE_ARG(::Engine::property<Ty, &Ty::Getter, nullptr>()))

#define READONLY_PROPERTY(Name, Getter) \
    READONLY_PROPERTY_EX(, Name, Getter)

#define PROPERTY_EX(Idx, Name, Getter, Setter) \
    METATABLE_ENTRY_EX(Idx, #Name, SINGLE_ARG(::Engine::property<Ty, &Ty::Getter, &Ty::Setter>()))

#define PROPERTY(Name, Getter, Setter) \
    PROPERTY_EX(, Name, Getter, Setter)

#define NAMED_FUNCTION_EX(Idx, Name, F, ...)                                                                                                                                                                                              \
    FUNCTIONTABLE_EX(BASE_STRUCT(::Engine::MetaTableTag, Idx)::name + "::" STRINGIFY(Name), ::Engine::MetaMemberFunctionTag<BASE_STRUCT(::Engine::MetaTableTag, Idx)::Ty>, BASE_STRUCT(::Engine::MetaTableTag, Idx)::Ty::F, #__VA_ARGS__) \
    METATABLE_ENTRY_EX(Idx, STRINGIFY(Name), SINGLE_ARG(::Engine::property<Ty, &::Engine::method<&Ty::F>, nullptr>()))

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
