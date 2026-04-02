#pragma once

#include "Generic/container/virtualrange.h"
#include "Generic/execution/binding.h"
#include "Generic/execution/concepts.h"
#include "Generic/keyvalue.h"

#include "../enumholder.h"
#include "../flagsholder.h"
#include "keyvaluebinding.h"
#include "keyvaluesender.h"
#include "ownedscopeptr.h"
#include "valuetype_desc.h"

namespace Engine {

META_EXPORT ValueType &KeyValuePair_key(KeyValuePair &p);
META_EXPORT ValueType &KeyValuePair_value(KeyValuePair &p);

template <typename T>
void to_ValueType(ValueType &v, T &&t);

template <typename T>
void to_KeyValuePair(KeyValuePair &p, T &&t)
{
    to_ValueType(KeyValuePair_key(p), kvKey(t));
    to_ValueType(KeyValuePair_value(p), kvValue(forward_ref<T>(t)));
}

struct Functor_to_KeyValuePair {
    template <typename... Args>
    decltype(auto) operator()(Args &&...args)
    {
        return to_KeyValuePair(std::forward<Args>(args)...);
    }
};

struct Functor_to_ValueType {
    template <typename... Args>
    decltype(auto) operator()(Args &&...args)
    {
        return to_ValueType(forward_ref<Args>(args)...);
    }
};

META_EXPORT const ValueType &getArgument(const ArgumentList &args, size_t index);

template <typename T>
using ValueTypePrimitiveSubList = ValueTypeList::select<ValueTypeList::index<size_t, T>>;

template <typename T>
using QualifiedValueTypePrimitiveSubList = QualifiedValueTypeList::select<ValueTypeList::index<size_t, T>>;

template <typename T>
struct ValueType_ReturnHelper {
    typedef T &type;
};

template <ValueTypePrimitive T>
struct ValueType_ReturnHelper<T> {
    typedef typename QualifiedValueTypePrimitiveSubList<T>::template select<ValueTypePrimitiveSubList<T>::template index<size_t, T>> type;
};

template <typename T>
struct ValueType_ReturnHelper<T *> {
    typedef T *type;
};

template <Execution::AnyBinding T>
    requires(!std::same_as<T, KeyValueBinding>)
struct ValueType_ReturnHelper<T> {
    typedef Execution::CallBinding<typename ValueType_ReturnHelper<typename T::type>::type (*)(const ValueType &), Execution::BindingPtr<const ValueType &>> type;
};

template <>
struct ValueType_ReturnHelper<ValueType> {
    typedef ValueType type;
};

template <typename T>
using ValueType_Return = typename ValueType_ReturnHelper<T>::type;

META_EXPORT bool ValueType_isNull(const ValueType &v);
META_EXPORT ValueTypeDesc ValueType_type(const ValueType &v);

template <typename T>
bool ValueType_is(const ValueType &v)
{
    return toValueTypeDesc<T>().canAccept(ValueType_type(v));
}

template <ValueTypePrimitive T>
META_EXPORT ValueType_Return<T> ValueType_as(const ValueType &v);

template <typename Callable, typename Arg>
KeyValueResult ValueType_call(Callable &&callable, Arg &&arg)
{    
    using T = meta_decayed_t<std::decay_t<typename CallableTraits<Callable>::argument_types::template unpack_unique<>>>;

    if constexpr (InstanceOf<T, std::optional>) {
        if (ValueType_isNull(arg))
            return callable(T {});
        else {
            return ValueType_call([&](const typename is_instance<T, std::optional>::argument_types::template unpack_unique<> &v) -> decltype(auto) {
                return callable(T { v });
            },
                std::forward<Arg>(arg));
        }
    } else if constexpr (InstanceOf<T, std::variant>) {
        throw 0;
    } else if constexpr (std::same_as<T, ValueType>) {
        return callable(arg);
    } else if constexpr (ValueTypePrimitive<T>) {
        return callable(ValueType_as<T>(arg));
    } else if constexpr (std::ranges::range<T> && requires { typename T::iterator; }) {
        if constexpr (std::same_as<KeyType_t<typename T::iterator::value_type>, Void>) {
            if (!ValueType_is<KeyValueVirtualSequenceRange>(arg))
                throw 0;
            return callable(ValueType_as<KeyValueVirtualSequenceRange>(arg).template safe_cast<T>());
        } else {
            if (!ValueType_is<KeyValueVirtualAssociativeRange>(arg))
                throw 0;
            return callable(ValueType_as<KeyValueVirtualAssociativeRange>(arg).template safe_cast<T>());
        }
    } else if constexpr (InstanceOf<std::decay_t<T>, EnumImpl>) {
        if (!ValueType_is<EnumHolder>(arg))
            throw 0;
        return callable(ValueType_as<EnumHolder>(arg).template safe_cast<T>());
    } else if constexpr (InstanceOf<std::decay_t<T>, Flags>) {
        if (!ValueType_is<FlagsHolder>(arg))
            throw 0;
        return callable(ValueType_as<FlagsHolder>(arg).template safe_cast<T>());
    } else if constexpr (Execution::AnyBinding<T>) {
        if (!ValueType_is<KeyValueBinding>(arg))
            throw 0;
        return callable(T { ValueType_as<KeyValueBinding>(arg).template unwrap<T>() });
    } else {
        if (ValueType_is<KeyValueBinding>(arg)) {
            KeyValueResult result;
            if (!Execution::access_binding(ValueType_as<KeyValueBinding>(arg), [&](const ValueType& v) {
                result = ValueType_call(std::forward<Callable>(callable), v);
                })) {
                throw 0;
            }
            return result;
        } else if (ValueType_is<ScopePtr>(arg)) {            
            ScopePtr scope = ValueType_as<ScopePtr>(arg);
            std::remove_pointer_t<T> *ptr = scope_cast<std::remove_pointer_t<T>>(scope);
            if (!ptr) {
                return KEYVALUE_UNKNOWN_ERROR() << "No known conversion from " << scope.mType->mTypeName << " to " << toValueTypeDesc<T>().toString();
            }
            if constexpr (std::is_pointer_v<T>) {
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
KeyValueResult ValueType_unwrap_impl(type_pack<>, Callable &&callable)
{
    return std::invoke(std::forward<Callable>(callable));
}

template <typename Callable, typename Arg, typename... Args, typename Param, typename... Params>
KeyValueResult ValueType_unwrap_impl(type_pack<Param, Params...>, Callable &&callable, Arg &&arg, Args &&...args)
{
    auto call = [&](Param param) {
        return ValueType_unwrap_impl(type_pack<Params...> {}, [&](Params... params) { return std::invoke(std::forward<Callable>(callable), std::forward<Param>(param), std::forward<Params>(params)...); }, std::forward<Args>(args)...);
    };

    return ValueType_call(call, std::forward<Arg>(arg));
}

template <typename Callable, typename... Args>
KeyValueResult ValueType_unwrap(ValueType &result, Callable &&callable, Args &&...args)
{
    using traits = CallableTraits<Callable>;

    if constexpr (std::same_as<typename traits::argument_types::template resize<1>, type_pack<ValueType &>>) {
        if constexpr (std::same_as<typename traits::class_type, void>) {
            return ValueType_unwrap_impl(typename traits::argument_types::pop_front {}, [&](auto &&...args) { return std::invoke(std::forward<Callable>(callable), result, std::forward<decltype(args)>(args)...); }, std::forward<Args>(args)...);
        } else {
            return ValueType_unwrap_impl(typename traits::argument_types::pop_front::template prepend<typename traits::class_type&> {}, [&](auto &&obj, auto &&...args) { return std::invoke(std::forward<Callable>(callable), obj, result, std::forward<decltype(args)>(args)...); }, std::forward<Args>(args)...);
        }
    } else {
        using argumentTypes = std::conditional_t<std::same_as<typename traits::class_type, void>,
            typename traits::argument_types,
            typename traits::argument_types::template prepend<std::add_lvalue_reference_t<typename traits::class_type>>>;
        return ValueType_unwrap_impl(argumentTypes {}, [&](auto &&...args) -> KeyValueResult {
            using R = std::invoke_result_t<Callable&&, decltype(args)&&...>;
            if constexpr (std::is_void_v<R>) {
                std::invoke(std::forward<Callable>(callable), std::forward<decltype(args)>(args)...);
            } else {
                to_ValueType(result, forward_ref<R>(std::invoke(std::forward<Callable>(callable), std::forward<decltype(args)>(args)...)));
            }            
            return {}; }, std::forward<Args>(args)...);
    }
}

template <typename Callable, typename... Args>
KeyValueResult ValueType_unwrap(Callable &&callable, Args &&...args)
{
    KeyValueResult result;
    ValueType_erased([&](ValueType &v) {
        result = ValueType_unwrap(v, std::forward<Callable>(callable), std::forward<Args>(args)...);
    });
    return result;
}

template <typename T>
    requires(ValueTypePrimitive<std::decay_t<T>> || std::same_as<ValueType, std::decay_t<T>>)
META_EXPORT void to_ValueType_impl(ValueType &v, T &&t);

template <typename... V>
void to_ValueType_impl(ValueType &v, std::variant<V...> &&t)
{
    std::visit([&v](auto &&arg) {
        to_ValueType(v, std::forward<decltype(arg)>(arg));
    },
        std::move(t));
}

template <typename T>
void to_ValueType(ValueType &v, T &&t)
{
    if constexpr (has_function_customScopePtr_v<std::remove_pointer_t<T>>) {
        to_ValueType_impl(v, convert_ValueType(resolveCustomScopePtr(std::forward<T>(t))));
    } else {
        to_ValueType_impl(v, convert_ValueType(std::forward<T>(t)));
    }
}

struct ValueTypeRef {

    ValueTypeRef(ValueType &ref)
        : mRef(ref)
    {
    }

    operator ValueType &()
    {
        return mRef;
    }

    template <typename T>
    ValueTypeRef &operator=(T &&v)
    {
        to_ValueType(mRef, std::forward<T>(v));
        return *this;
    }

private:
    ValueType &mRef;
};
}