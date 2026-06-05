#pragma once

#include "Generic/containers/virtualrange.h"
#include "Generic/cow.h"
#include "Generic/cowstring.h"
#include "Generic/execution/binding.h"
#include "Generic/execution/concepts.h"
#include "Generic/keyvalue.h"

#include "enum.h"
#include "flags.h"
#include "binding.h"
#include "sender.h"
#include "ownedscopeptr.h"
#include "type.h"

namespace Engine {
namespace Reflect {

    META_EXPORT Value &KeyValuePair_key(KeyValuePair &p);
    META_EXPORT Value &KeyValuePair_value(KeyValuePair &p);

    template <typename T>
    void toValue(Value &v, T &&t);

    template <typename T>
    void toKeyValuePair(KeyValuePair &p, T &&t)
    {
        toValue(KeyValuePair_key(p), kvKey(t));
        toValue(KeyValuePair_value(p), kvValue(forward_ref<T>(t)));
    }

    struct Functor_toKeyValuePair {
        template <typename... Args>
        decltype(auto) operator()(Args &&...args)
        {
            return toKeyValuePair(std::forward<Args>(args)...);
        }
    };

    struct Functor_toValue {
        template <typename... Args>
        decltype(auto) operator()(Args &&...args)
        {
            return toValue(forward_ref<Args>(args)...);
        }
    };

    META_EXPORT const Value &getArgument(const ArgumentList &args, size_t index);

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
        } else if constexpr (PrimitiveType<T>) {
            if (!Value_is<T>(arg))
                return REFLECT_UNKNOWN_ERROR() << "Expected " << typeid(T).name();
            return callable(Value_as<ValueStorageSelect<T>>(arg));
        } else if constexpr (std::ranges::range<T> && requires { typename T::iterator; }) {
            if constexpr (std::same_as<KeyType_t<typename T::iterator::value_type>, Void>) {
                if (!Value_is<SequenceRange>(arg))
                    throw 0;
                return callable(Value_as<SequenceRange>(arg).template safe_cast<T>());
            } else {
                if (!Value_is<AssociativeRange>(arg))
                    throw 0;
                return callable(Value_as<AssociativeRange>(arg).template safe_cast<T>());
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
            } else if (Value_is<ScopePtr>(arg)) {
                using Ty = resolveCustomScopePtr_t<T, true>;
                ScopePtr scope = Value_as<ScopePtr>(arg);
                std::remove_pointer_t<Ty> *ptr = scope_cast<std::remove_pointer_t<Ty>>(scope);
                if (!ptr) {
                    return REFLECT_UNKNOWN_ERROR() << "No known conversion from " << scope.mType->mTypeName << " to " << toType<Ty>().toString();
                }
                if constexpr (std::is_pointer_v<Ty>) {
                    return callable(ptr);
                } else {
                    return callable(*ptr);
                }
            }
            throw 0;
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

        if constexpr (std::same_as<typename traits::argument_types::template resize<1>, type_pack<Value&>>) {
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