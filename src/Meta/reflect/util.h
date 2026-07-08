#pragma once

#include "Generic/containers/virtualrange.h"
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
#include "ownedscopeptr.h"
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
                using Inner = decltype(convert_Value_t<false> {}(std::declval<forward_ref_t<typename std::decay_t<T>::type>>()));
                if constexpr (Concepts::OneOf<Inner, ScopePtr, OwnedScopePtr>) {
                    return ScopeBinding { std::forward<T>(t), table<std::remove_pointer_t<std::decay_t<typename std::decay_t<T>::type>>> };
                } else {
                    return Binding { std::forward<T>(t), toTypeIndex<std::decay_t<typename std::decay_t<T>::type>>() };
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
                return OwnedScopePtr { std::forward<T>(t) };
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
            requires tag_invocable<convert_Value_t, T>
        decltype(auto) operator()(T &&t) const
        {
            return tag_invoke(*this, std::forward<T>(t));
        }
    };

    inline constexpr convert_Value_t<false> convert_Value {};

    template <typename T, typename Callable>
    Result callSequenceRange(T output, SequenceIterator it, Callable &&callable)
    {
        if (it.ended()) {
            return callable(std::forward<T>(output));
        } else {
            Result result;
            (it++).get([&](const Value &v) {
                result = call([&](T::value_type v) {
                    output.emplace_back(std::move(v));
                    return callSequenceRange(std::move(output), std::move(it), std::forward<Callable>(callable));
                },
                    v);
            });
            return result;
        }
    }

    template <typename Callable, typename Arg>
    Result call(Callable &&callable, Arg &&arg)
    {
        using T = meta_decayed_t<std::decay_t<typename CallableTraits<Callable>::argument_types::template unpack_unique<>>>;

        if constexpr (Concepts::InstanceOf<T, std::optional>) {
            if (Value_isNull(arg))
                return callable(T {});
            else {
                return call([&](const typename Concepts::is_instance<T, std::optional>::argument_types::template unpack_unique<> &v) -> decltype(auto) {
                    return callable(T { v });
                },
                    std::forward<Arg>(arg));
            }
        } else if constexpr (Concepts::InstanceOf<T, std::variant>) {
            return [&]<typename... Ty>(type_pack<Ty...>) {
                bool matched = false;
                Result result;
                ([&]() {
                    if (Value_is<Ty>(arg)) {
                        if (matched) {
                            result = REFLECT_UNKNOWN_ERROR() << "More than one variant type could match";
                            return;
                        }
                        matched = true;
                        result = call([&](const Ty &t) {
                            return callable(T { t });
                        },
                            std::forward<Arg>(arg));
                    }
                }(),
                    ...);
                if (!matched) {
                    result = REFLECT_UNKNOWN_ERROR() << "No variant type matched argument";
                }
                return result;
            }(typename Concepts::is_instance<T, std::variant>::argument_types {});
        } else if constexpr (std::same_as<T, Value>) {
            return callable(arg);
        } else if constexpr (std::same_as<T, ScopePtr>) {
            if (Value_is<ScopePtr>(arg)) {
                return callable(Value_as<ScopePtr>(arg));
            } else if (Value_is<OwnedScopePtr>(arg)) {
                return callable(Value_as<OwnedScopePtr>(arg).get());
            } else {
                return REFLECT_UNKNOWN_ERROR() << "Cannot form a scope pointer to type " << Value_type(arg).toString();
            }
        } else if constexpr (PrimitiveType<T>) {
            if (!Value_is<T>(arg))
                return REFLECT_UNKNOWN_ERROR() << "Expected " << typeid(T).name();
            return callable(Value_as<ValueStorageSelect<T>>(arg));
        } else if constexpr (std::ranges::range<T> && requires { typename T::iterator; }) {
            if constexpr (std::same_as<KeyType_t<typename T::iterator::value_type>, Void>) {
                if (!Value_is<SequenceRange>(arg))
                    throw 0;

                return callSequenceRange(T {}, Value_as<SequenceRange>(arg).begin(), std::forward<Callable>(callable));
            } else {
                if (!Value_is<AssociativeRange>(arg))
                    throw 0;

                return callAssociatveRange(T {}, Value_as<AssociativeRange>(arg).begin(), std::forward<Callable>(callable));
            }
        } else if constexpr (Concepts::InstanceOf<std::decay_t<T>, EnumImpl>) {
            if (!Value_is<Enum>(arg))
                throw 0;
            return callable(Value_as<Enum>(arg).template safe_cast<T>());
        } else if constexpr (Concepts::InstanceOf<std::decay_t<T>, Engine::Flags>) {
            if (!Value_is<Flags>(arg))
                throw 0;
            return callable(Value_as<Flags>(arg).template safe_cast<T>());
        } else if constexpr (Execution::AnyBinding<T>) {
            using Inner = decltype(convert_Value_t<false> {}(std::declval<forward_ref_t<typename std::decay_t<T>::type>>()));
            if constexpr (Concepts::OneOf<Inner, ScopePtr, OwnedScopePtr>) {
                if (!Value_is<ScopeBinding>(arg))
                    return REFLECT_UNKNOWN_ERROR() << "No known conversion to ScopeBinding";
                return callable(T { Value_as<ScopeBinding>(arg).template unwrap<T>() });
            } else {
                if (!Value_is<Binding>(arg))
                    return REFLECT_UNKNOWN_ERROR() << "No known conversion to Binding";
                return callable(T { Value_as<Binding>(arg).template unwrap<T>() });
            }
        } else {
            if (Value_is<Binding>(arg)) {
                Result result;
                if (!Execution::access_binding(Value_as<Binding>(arg), [&](const Value &v) {
                        result = call(std::forward<Callable>(callable), v);
                    })) {
                    throw 0;
                }
                return result;
            } else if (Value_is<ScopeBinding>(arg)) {
                Result result;
                if (!Execution::access_binding(Value_as<ScopeBinding>(arg), [&](const Value &v) {
                        result = call(std::forward<Callable>(callable), v);
                    })) {
                    throw 0;
                }
                return result;
            } else {
                return call([&](ScopePtr scope) -> Result {
                    using Ty = resolveCustomScopePtr_t<T, true>;
                    std::remove_pointer_t<Ty> *ptr = scope_cast<std::remove_pointer_t<Ty>>(scope);
                    if (!ptr) {
                        return REFLECT_UNKNOWN_ERROR() << "No known conversion from " << scope.mType->mTypeName << " to " << toType<Ty>().toString();
                    }
                    if constexpr (std::is_pointer_v<Ty>) {
                        return callable(ptr);
                    } else {
                        return callable(*ptr);
                    }
                },
                    arg);
            }

            /*using U = resolveCustomScopePtr_t<std::remove_reference_t<T>, true>;
            std::remove_pointer_t<U> *ptr = scope_cast<std::remove_pointer_t<U>>(ValueType_as_impl<ScopePtr>(v));
            if constexpr (Pointer<U>) {
                return ptr;
            } else {
                return *ptr;
            }*/
        }
        // static_assert(dependent_bool<T, false>::value, "A ValueType can not be converted to the given target type");
    }

    template <typename Callable>
    Result invoke_impl(type_pack<>, Callable &&callable)
    {
        return std::invoke(std::forward<Callable>(callable));
    }

    template <typename Callable, typename Arg, typename... Args, typename Param, typename... Params>
    Result invoke_impl(type_pack<Param, Params...>, Callable &&callable, Arg &&arg, Args &&...args)
    {
        auto tail = [&](Param param) {
            return invoke_impl(type_pack<Params...> {}, [&](Params... params) { return std::invoke(std::forward<Callable>(callable), std::forward<Param>(param), std::forward<Params>(params)...); }, std::forward<Args>(args)...);
        };

        return call(tail, std::forward<Arg>(arg));
    }

    template <typename Callable, typename... Args>
    Result invoke(Value &result, Callable &&callable, Args &&...args)
    {
        using traits = CallableTraits<Callable>;

        if constexpr (std::same_as<typename traits::argument_types::template resize<1>, type_pack<Value &>>) {
            if constexpr (std::same_as<typename traits::class_type, void>) {
                return invoke_impl(typename traits::argument_types::pop_front {}, [&](auto &&...args) { return std::invoke(std::forward<Callable>(callable), result, std::forward<decltype(args)>(args)...); }, std::forward<Args>(args)...);
            } else {
                return invoke_impl(typename traits::argument_types::pop_front::template prepend<typename traits::class_type &> {}, [&](auto &&obj, auto &&...args) { return std::invoke(std::forward<Callable>(callable), obj, result, std::forward<decltype(args)>(args)...); }, std::forward<Args>(args)...);
            }
        } else {
            using argumentTypes = std::conditional_t<std::same_as<typename traits::class_type, void>,
                typename traits::argument_types,
                typename traits::argument_types::template prepend<std::add_lvalue_reference_t<typename traits::class_type>>>;
            return invoke_impl(argumentTypes {}, [&](auto &&...args) -> Result {
            using R = std::invoke_result_t<Callable&&, decltype(args)&&...>;
            if constexpr (std::is_void_v<R>) {
                std::invoke(std::forward<Callable>(callable), std::forward<decltype(args)>(args)...);
            } else {
                toValue(result, forward_ref<R>(std::invoke(std::forward<Callable>(callable), std::forward<decltype(args)>(args)...)));
            }            
            return {}; }, std::forward<Args>(args)...);
        }
    }

    template <typename Callable, typename... Args>
    Result invoke(Callable &&callable, Args &&...args)
    {
        Result result;
        Value_erased([&](Value &v) {
            result = invoke(v, std::forward<Callable>(callable), std::forward<Args>(args)...);
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