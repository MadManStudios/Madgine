#pragma once

#include "Generic/closure.h"
#include "Generic/containers/virtualrange.h"
#include "Generic/context.h"
#include "Generic/cow.h"
#include "Generic/cowstring.h"
#include "Generic/execution/binding.h"
#include "Generic/execution/concepts.h"
#include "Generic/keyvalue.h"
#include "Generic/offsetptr.h"

#include "binding.h"
#include "enum.h"
#include "flags.h"
#include "metatable.h"
#include "ownedvalue.h"
#include "scopeptr.h"
#include "sender.h"
#include "type.h"

namespace Engine {
namespace Reflect {

    template <typename T>
        requires(!std::is_pointer_v<T>)
    T *scope_cast(const ScopePtr &ptr)
    {
        OffsetPtr offset { 0 };
        if (!ptr.mType->isDerivedFrom<std::remove_const_t<T>>(&offset))
            return nullptr;

        return static_cast<T *>(reinterpret_cast<void *>(reinterpret_cast<std::byte *>(ptr.mScope) + offset));
    }

    template <typename F, typename R, typename T, typename... Args>
    struct DynamicCastHelper {

        ScopePtr operator()(T &t, Args &&...args) const
        {
            R result = mCallable(t, std::forward<Args>(args)...);

            return { result, mType };
        }

        const MetaTable *dynamic_return_type() const
        {
            return mType;
        }

        const MetaTable *mType;
        F mCallable;
    };

    template <typename F, typename R, typename... Args>
    struct DynamicCastHelper<F, R, void, Args...> {

        ScopePtr operator()(Args &&...args) const
        {
            R result = mCallable(std::forward<Args>(args)...);

            return { result, mType };
        }

        const MetaTable *dynamic_return_type() const
        {
            return mType;
        }

        const MetaTable *mType;
        F mCallable;
    };

    template <typename F>
    auto dynamic_scope_cast(const MetaTable *type, F &&f)
    {
        return typename CallableTraits<F>::template instance<DynamicCastHelper, F> { type, std::forward<F>(f) };
    }

    template <typename T>
    void toValue(Value &v, T &&t);

    struct VirtualRangeHelper {
        template <typename T>
        void operator()(CallableView<void(const Value &)> cb, T &&t)
        {
            Value_erased([&](Value &v) {
                toValue(v, forward_ref<T>(t));
                cb(v);
            });
        }

        template <typename T>
        void operator()(CallableView<void(const Value &, const Value &)> cb, T &&t)
        {
            Value_erased([&](Value &key) {
                Value_erased([&](Value &value) {
                    toValue(key, kvKey(t));
                    toValue(value, kvValue(forward_ref<T>(t)));
                    cb(key, value);
                });
            });
        }
    };

    META_EXPORT const Value &getArgument(const ArgumentList &args, size_t index);
    META_EXPORT size_t argumentCount(const ArgumentList &args);

    template <typename T>
    using ValueStorageSelect = ValueStorageList::select<TypeList::index<size_t, T>>;

    META_EXPORT bool Value_isNull(const Value &v);
    META_EXPORT Type Value_type(const Value &v);

    template <typename T>
    bool Value_is(const Value &v)
    {
        return toType<T>().canAccept(Value_type(v));
    }

    template <ValueStorage T>
    META_EXPORT const T &Value_as(const Value &v);
    template <ValueStorage T>
    META_EXPORT T &Value_as(Value &v);

    template <bool isReferenceWrapped>
    struct convert_Value_t {

        template <typename T>
            requires(!tag_invocable<convert_Value_t, T>)
        decltype(auto) operator()(T &&t) const
        {
            static_assert(!requires { typename std::decay_t<T>::no_value_type; });

            if constexpr (Concepts::InstanceOf<std::decay_t<T>, std::reference_wrapper>) {
                return convert_Value_t<true> {}(t.get());
            } else if constexpr (PrimitiveType<std::decay_t<T>> || std::same_as<Value, std::decay_t<T>>) {
                return std::forward<T>(t);
            } else if constexpr (Concepts::String<std::decay_t<T>>) {
                return std::string { std::forward<T>(t) };
            } else if constexpr (std::ranges::range<T>) {
                if constexpr (std::same_as<KeyType_t<std::ranges::range_value_t<T>>, Void>)
                    return SequenceRange { std::forward<T>(t), type_holder<VirtualRangeHelper> };
                else
                    return AssociativeRange { std::forward<T>(t) };
            } else if constexpr (std::is_enum_v<std::decay_t<T>>) {
                if constexpr (std::is_reference_v<T>) {
                    return static_cast<std::underlying_type_t<T> &>(t);
                } else {
                    return static_cast<std::underlying_type_t<T>>(t);
                }
            } else if constexpr (Concepts::InstanceOf<std::decay_t<T>, EnumImpl>) {
                return Enum { std::forward<T>(t) };
            } else if constexpr (Concepts::InstanceOf<std::decay_t<T>, Engine::Flags>) {
                return Flags { std::forward<T>(t) };
            } else if constexpr (Execution::AnyBinding<std::decay_t<T>>) {
                constexpr ExtendedType type = toType<forward_ref_t<typename std::decay_t<T>::type>>();
                if constexpr (type.mType == TypeEnum::ScopeValue || type.mType == TypeEnum::OwnedValueValue) {
                    return ScopeBinding { std::forward<T>(t), type.mSecondary.mMetaTable ? *type.mSecondary.mMetaTable : nullptr };
                } else {
                    return Binding { std::forward<T>(t), type.mType };
                }
            } else if constexpr (Execution::AnySender<std::decay_t<T>>) {
                return Sender { std::forward<T>(t) };
            } else if constexpr (Concepts::InstanceOfA<std::decay_t<T>, TypedBoundApiFunction>) {
                return BoundApiFunction { std::forward<T>(t) };
            } else if constexpr (Concepts::Pointer<std::decay_t<T>>) {
                return ScopePtr { t };
            } else if constexpr (Concepts::InstanceOf<std::decay_t<T>, std::unique_ptr>) {
                return ScopePtr { t.get() };
            } else if constexpr (isReferenceWrapped) {
                return ScopePtr { &t };
            } else {
                return OwnedValue { std::forward<T>(t) };
            }
            // static_assert(dependent_bool<T, false>::value, "The provided type can not be converted to a ValueType");
        }

        template <typename T>
        friend std::variant<T, std::monostate> tag_invoke(convert_Value_t, std::optional<T> &&o)
        {
            if (o) {
                return { std::move(*o) };
            } else {
                return { std::monostate {} };
            }
        }

        template <typename T>
        friend std::variant<std::reference_wrapper<T>, std::monostate> tag_invoke(convert_Value_t, std::optional<T> &o)
        {
            if (o) {
                return { *o };
            } else {
                return { std::monostate {} };
            }
        }

        template <typename T>
        friend std::variant<std::reference_wrapper<const T>, std::monostate> tag_invoke(convert_Value_t, const std::optional<T> &o)
        {
            if (o) {
                return { *o };
            } else {
                return { std::monostate {} };
            }
        }

        template <typename T>
            requires tag_invocable<convert_Value_t, T>
        decltype(auto) operator()(T &&t) const
        {
            return tag_invoke(*this, std::forward<T>(t));
        }
    };

    inline constexpr convert_Value_t<false> convert_Value {};

    template <typename T, typename Callable, typename Context>
    Result callSequenceRange(T output, SequenceIterator it, Callable &&callable, Context &&context)
    {
        if (it.ended()) {
            return callable(std::forward<T>(output), context);
        } else {
            Result result;
            (it++).get([&](const Value &v) {
                result = call([&](T::value_type v) {
                    output.emplace_back(std::move(v));
                    return callSequenceRange(std::move(output), std::move(it), std::forward<Callable>(callable), context);
                },
                    v, context);
            });
            return result;
        }
    }

    template <typename T>
    struct call_t { };

    template <typename T, typename Callable, typename Context>
    Result tag_invoke(call_t<T> call, Callable &&callable, const Value &arg, Context &&context)
    {
        if (Value_is<OwnedValue>(arg)) {
            Result result;
            Value_erased([&](Value &v) {
                Value_as<OwnedValue>(arg).get(v);
                result = tag_invoke(call, std::forward<Callable>(callable), v, context);
            });
            return result;
        }

        if constexpr (Concepts::InstanceOf<T, std::optional>) {
            if (Value_isNull(arg))
                return callable(T {}, context);
            else {
                return tag_invoke(call_t<typename T::value_type> {}, [&](const typename T::value_type &v, Context &context) -> decltype(auto) { return callable(T { v }, context); }, arg, context);
            }
        } else if constexpr (Concepts::InstanceOf<T, std::variant>) {
            return [&]<typename... Ty>(type_pack<Ty...>) {
                bool matched = false;
                Result result;
                ([&]() {
                    if (Value_is<Ty>(arg)) {
                        if (matched) {
                            result = REFLECT_UNKNOWN_ERROR_NOTRACE() << "More than one variant type could match";
                            return;
                        }
                        matched = true;
                        result = tag_invoke(call_t<Ty> {}, [&](const Ty &t, Context &context) { return callable(T { t }, context); }, arg, context);
                    }
                }(),
                    ...);
                if (!matched) {
                    result = REFLECT_UNKNOWN_ERROR_NOTRACE() << "No variant type matched argument";
                }
                return result;
            }(typename Concepts::is_instance<T, std::variant>::argument_types {});
        } else if constexpr (std::same_as<T, Value>) {
            return callable(arg, context);
        } else if constexpr (PrimitiveType<T>) {
            if (!Value_is<T>(arg))
                return REFLECT_UNKNOWN_ERROR_NOTRACE() << "Expected " << typeid(T).name() << " got " << Value_type(arg).toString();
            return callable(Value_as<ValueStorageSelect<T>>(arg), context);
        } else if constexpr (std::ranges::range<T> && requires { typename T::iterator; }) {
            if constexpr (std::same_as<KeyType_t<typename T::iterator::value_type>, Void>) {
                if (!Value_is<SequenceRange>(arg))
                    throw 0;

                return callSequenceRange(T {}, Value_as<SequenceRange>(arg).begin(), callable, context);
            } else {
                if (!Value_is<AssociativeRange>(arg))
                    throw 0;

                return callAssociatveRange(T {}, Value_as<AssociativeRange>(arg).begin(), callable);
            }
        } else if constexpr (Concepts::InstanceOf<std::decay_t<T>, EnumImpl>) {
            if (!Value_is<Enum>(arg))
                throw 0;
            return callable(Value_as<Enum>(arg).template safe_cast<T>(), context);
        } else if constexpr (Concepts::InstanceOf<std::decay_t<T>, Engine::Flags>) {
            if (!Value_is<Flags>(arg))
                throw 0;
            return callable(Value_as<Flags>(arg).template safe_cast<T>(), context);
        } else if constexpr (Execution::AnyBinding<T>) {
            using Inner = decltype(convert_Value_t<false> {}(std::declval<forward_ref_t<typename std::decay_t<T>::type>>()));
            if constexpr (Concepts::OneOf<Inner, ScopePtr, OwnedValue>) {
                if (!Value_is<ScopeBinding>(arg))
                    return REFLECT_UNKNOWN_ERROR_NOTRACE() << "No known conversion from " << Value_type(arg).toString() << " to ScopeBinding";
                return callable(T { Value_as<ScopeBinding>(arg).template typed<typename T::type>() }, context);
            } else {
                if (!Value_is<Binding>(arg))
                    return REFLECT_UNKNOWN_ERROR_NOTRACE() << "No known conversion from " << Value_type(arg).toString() << " to Binding";
                return callable(T { Value_as<Binding>(arg).template typed<typename T::type>() }, context);
            }
        } else if constexpr (Concepts::InstanceOf<std::decay_t<T>, Engine::__generic_impl__::ClosureImpl>) {
            if (Value_isNull(arg)) {
                return callable(T {}, context);
            } else {
                throw 0;
            }
            throw 0;
        } else {
            if (Value_is<Binding>(arg)) {
                return Value_as<Binding>(arg).access([&](const Value &v, auto &&context) {
                    return tag_invoke(call, std::forward<Callable>(callable), v, context);
                },
                    context);
            } else if (Value_is<ScopeBinding>(arg)) {
                return Value_as<ScopeBinding>(arg).access([&](const Value &v, auto &&context) {
                    return tag_invoke(call, std::forward<Callable>(callable), v, context);
                },
                    context);
            } else {
                if (Value_is<ScopePtr>(arg)) {
                    using Ty = resolveCustomScopePtr_t<T, true>;
                    std::remove_pointer_t<Ty> *ptr = scope_cast<std::remove_pointer_t<Ty>>(Value_as<ScopePtr>(arg));
                    if (ptr || (!Value_as<ScopePtr>(arg).mScope && std::is_pointer_v<Ty>)) {
                        if constexpr (std::is_pointer_v<Ty>) {
                            return callable(ptr, context);
                        } else {
                            return callable(*ptr, context);
                        }    
                    }                    
                }

                return REFLECT_UNKNOWN_ERROR_NOTRACE() << "No known conversion from " << Value_type(arg).toString() << " to " << toType<T>().toString();
            }
        }
        // static_assert(dependent_bool<T, false>::value, "A ValueType can not be converted to the given target type");
    }

    template <typename Callable, typename Arg, typename Context = ContextPtr>
    Result call(Callable &&_callable, Arg &&arg, Context &&context = {})
    {
        using traits = CallableTraits<Callable>;
        using argument_types = typename traits::argument_types;

        using T = meta_decayed_t<std::decay_t<typename argument_types::template select<0>>>;

        static_assert(argument_types::size == 2 || argument_types::size == 1);

        auto callable = [&](auto &&v, Context &context) -> Result {
            if constexpr (argument_types::size == 1) {
                return _callable(std::forward<decltype(v)>(v));
            } else {
                static_assert(std::convertible_to<Context &, typename argument_types::template select<1>>);
                return _callable(std::forward<decltype(v)>(v), context);
            }
        };

        return tag_invoke(call_t<T> {}, std::move(callable), std::forward<Arg>(arg), std::forward<Context>(context));
    }

    template <typename Callable, typename Context>
    Result invoke_impl(type_pack<>, size_t index, Callable &&callable, Context &&context)
    {
        return std::invoke(std::forward<Callable>(callable));
    }

    template <typename Callable, typename Context, typename Arg, typename... Args, typename Param, typename... Params>
        requires(!Concepts::InstanceOf<Param, Contextual>)
    Result invoke_impl(type_pack<Param, Params...>, size_t index, Callable &&callable, Context &&context, Arg &&arg, Args &&...args)
    {
        bool passed = false;

        auto tail = [&](Param param) {
            passed = true;
            return invoke_impl(type_pack<Params...> {}, index + 1, [&](Params... params) { return std::invoke(std::forward<Callable>(callable), std::forward<Param>(param), std::forward<Params>(params)...); }, context, std::forward<Args>(args)...);
        };

        Result result = call(tail, std::forward<Arg>(arg), context);
        if (result && !passed) {
            result.mError->mMsg += "\nnote: in parameter " + std::to_string(index);
        }
        return result;
    }

    template <typename Callable, typename Context, typename Param, typename... Params>
    Result invoke_impl(type_pack<Contextual<Param>, Params...>, size_t index, Callable &&callable, Context &&context)
    {
        using T = std::remove_reference_t<Param>;
        T *ptr = context_get<T>(context);

        if (!ptr)
            throw 0;

        return invoke_impl(type_pack<Params...> {}, index + 1, [&](Params... params) { return std::invoke(std::forward<Callable>(callable), *ptr, std::forward<Params>(params)...); }, context);
    }

    template <typename Callable, typename... Args, typename Context = ContextPtr>
    Result invoke_free(Value &result, Callable &&callable, Context &&context = {}, Args &&...args)
    {
        using traits = CallableTraits<Callable>;

        if constexpr (std::same_as<typename traits::argument_types::template resize<1>, type_pack<Value &>>) {
            return invoke_impl(typename traits::argument_types::pop_front {}, 1, [&](auto &&...args) { return std::invoke(std::forward<Callable>(callable), result, std::forward<decltype(args)>(args)...); }, context, std::forward<Args>(args)...);
        } else {
            return invoke_impl(typename traits::argument_types {}, 1, [&](auto &&...args) -> Result {
            using R = std::invoke_result_t<Callable&&, decltype(args)&&...>;
            if constexpr (std::is_void_v<R>) {
                std::invoke(std::forward<Callable>(callable), std::forward<decltype(args)>(args)...);
            } else {
                toValue(result, forward_ref<R>(std::invoke(std::forward<Callable>(callable), std::forward<decltype(args)>(args)...)));
            }            
            return {}; }, context, std::forward<Args>(args)...);
        }
    }

    template <typename Callable, typename... Args, typename Context = ContextPtr>
    Result invoke_free(Callable &&callable, Context &&context = {}, Args &&...args)
    {
        Result result;
        Value_erased([&](Value &v) {
            result = invoke_free(v, std::forward<Callable>(callable), context, std::forward<Args>(args)...);
        });
        return result;
    }

    template <bool isMember, typename traits>
    struct invoke_member_helper {
        using class_type = typename traits::class_type;
        using argument_types = typename traits::argument_types;
    };

    template <typename traits>
    struct invoke_member_helper<false, traits> {
        static_assert(std::is_lvalue_reference_v<typename traits::argument_types::first> || std::is_pointer_v<typename traits::argument_types::first>, "First argument of a free member-like function must be a reference or a pointer");
        using class_type = std::remove_reference_t<typename traits::argument_types::first>;
        using argument_types = typename traits::argument_types::pop_front;
    };

    template <typename T, typename R, typename argument_types, typename Callable>
    struct AccessBindingForwarder : __Reflect_impl__::InnerBindingBase {

        using type = R;

        AccessBindingForwarder(type_pack<T, R, argument_types>, Callable &&callable, ScopeBinding scope)
            : mCallable(std::forward<Callable>(callable))
            , mScope(std::move(scope))
        {
        }

        template <typename F, typename Context>
        Result access(F &&callback, Context &&context) const
        {
            return mScope.typed<T>().access([&](T &t, Context &context) {
                return invoke_impl(argument_types {}, 1, [&](auto &&...args) { return callback(std::invoke(mCallable, t, std::forward<decltype(args)>(args)...), context_set(context, t)); }, context_set(context, t));
            },
                context);
        }

        Result access(CallableView<Result(const Value &, ContextPtr)> cb, ContextPtr context) const override
        {
            return access([&](auto &&t, ContextPtr context) {
                Result result;
                Value_erased([&](Value &v) {
                    toValue(v, forward_ref<decltype(t)>(t));
                    result = cb(v, context);
                });
                return result;
            },
                context);
        }

        Callable mCallable;
        ScopeBinding mScope;
    };

    template <typename T, typename R, typename argument_types, typename Callable>
    AccessBindingForwarder(type_pack<T, R, argument_types>, Callable &&, ScopeBinding) -> AccessBindingForwarder<T, R, argument_types, Callable>;

    template <typename Callable, typename Arg, typename... Args, typename Context = ContextPtr>
    Result invoke_member(Value &result, Callable &&callable, Context &&context, Arg &&arg, Args &&...args)
    {
        using traits = CallableTraits<Callable>;

        using T = typename invoke_member_helper<traits::is_member_function, traits>::class_type;
        using argument_types = typename invoke_member_helper<traits::is_member_function, traits>::argument_types;

        if constexpr (sizeof...(Args) == 0 && !std::same_as<typename traits::return_type, void> && (!PrimitiveType<std::decay_t<typename traits::return_type>> || std::same_as<ScopePtr, typename traits::return_type>)) {
            if (Value_is<ScopeBinding>(arg)) {

                constexpr ExtendedType type = toType<forward_ref_t<typename traits::return_type>>();
                if constexpr (type.mType == TypeEnum::ScopeValue || type.mType == TypeEnum::OwnedValueValue) {
                    const MetaTable *table;
                    if constexpr (type.mSecondary.mMetaTable) {
                        table = *type.mSecondary.mMetaTable;
                    } else {
                        table = callable.dynamic_return_type();
                    }
                    toValue(result, ScopeBinding { new AccessBindingForwarder { type_pack<std::remove_const_t<T>, typename traits::return_type, argument_types> {}, std::forward<Callable>(callable), Value_as<ScopeBinding>(arg) }, table });
                } else {
                    toValue(result, Binding { new AccessBindingForwarder { type_pack<std::remove_const_t<T>, typename traits::return_type, argument_types> {}, std::forward<Callable>(callable), Value_as<ScopeBinding>(arg) }, type.mType });
                }

                return {};
            }
        }

        return call([&](T &t, Context &context) {
            if constexpr (std::same_as<typename argument_types::template resize<1>, type_pack<Value &>>) {
                return invoke_impl(typename argument_types::pop_front {}, 1, [&](auto &&...args) { return std::invoke(std::forward<Callable>(callable), t, result, std::forward<decltype(args)>(args)...); }, context, std::forward<Args>(args)...);
            } else {
                return invoke_impl(argument_types {}, 1, [&](auto &&...args) -> Result {
            using R = std::invoke_result_t<Callable&&, T&, decltype(args)&&...>;
            if constexpr (std::is_void_v<R>) {
                std::invoke(std::forward<Callable>(callable), t, std::forward<decltype(args)>(args)...);
            } else {
                toValue(result, forward_ref<R>(std::invoke(std::forward<Callable>(callable), t, std::forward<decltype(args)>(args)...)));
            }            
            return {}; }, context, std::forward<Args>(args)...);
            } },
            std::forward<Arg>(arg), context);
    }

    template <typename Callable, typename Arg, typename... Args, typename Context = ContextPtr>
    Result invoke_member(Callable &&callable, Context &&context, Arg &&arg, Args &&...args)
    {
        Result result;
        Value_erased([&](Value &v) {
            result = invoke_member(v, std::forward<Callable>(callable), context, std::forward<Arg>(arg), std::forward<Args>(args)...);
        });
        return result;
    }

    template <typename T>
        requires(PrimitiveType<std::decay_t<T>> || std::same_as<Value, std::decay_t<T>>)
    META_EXPORT void toValue_impl(Value &v, T &&t);

    template <typename... V>
    void toValue_impl(Value &v, std::variant<V...> &&t)
    {
        std::visit([&v](auto &&arg) {
            toValue(v, std::forward<decltype(arg)>(arg));
        },
            std::move(t));
    }

    template <typename T>
    void toValue(Value &v, T &&t)
    {
        if constexpr (has_function_customScopePtr_v<std::remove_pointer_t<T>>) {
            toValue_impl(v, convert_Value(resolveCustomScopePtr(std::forward<T>(t))));
        } else {
            toValue_impl(v, convert_Value(std::forward<T>(t)));
        }
    }

}
}