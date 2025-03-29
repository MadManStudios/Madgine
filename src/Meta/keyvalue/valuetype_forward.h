#pragma once

#include "ownedscopeptr.h"

#include "keyvaluesender.h"

#include "Generic/container/virtualrange.h"

#include "Generic/keyvalue.h"

#include "valuetype_desc.h"

#include "../enumholder.h"

#include "../flagsholder.h"

#include "Generic/execution/concepts.h"

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
META_EXPORT ValueType_Return<T> ValueType_as_impl(const ValueType &v);

template <typename T>
decltype(auto) ValueType_as(const ValueType &v)
{
    if constexpr (InstanceOf<T, std::optional>) {
        if (ValueType_isNull(v))
            return T {};
        else {
            return T { ValueType_as<typename is_instance<T, std::optional>::argument_types::template unpack_unique<>>(v) };
        }
    } else if constexpr (std::same_as<T, ValueType>) {
        return v;
    } else if constexpr (ValueTypePrimitive<T>) {
        return ValueType_as_impl<T>(v);
    } else if constexpr (std::ranges::range<T>) {
        if constexpr (std::same_as<KeyType_t<typename T::iterator::value_type>, Void>)
            return ValueType_as_impl<KeyValueVirtualSequenceRange>(v).safe_cast<T>();
        else
            return ValueType_as_impl<KeyValueVirtualAssociativeRange>(v).safe_cast<T>();
    } else if constexpr (InstanceOf<std::decay_t<T>, EnumType> || InstanceOf<std::decay_t<T>, BaseEnum>) {
        return ValueType_as_impl<EnumHolder>(v).safe_cast<T>();
    } else if constexpr (InstanceOf<std::decay_t<T>, Flags>) {
        return ValueType_as_impl<FlagsHolder>(v).safe_cast<T>();
    } else {
        using Ty = resolveCustomScopePtr_t<T, true>;
        if constexpr (Pointer<Ty>) {
            return scope_cast<std::remove_pointer_t<Ty>>(ValueType_as_impl<ScopePtr>(v));
        } else {
            return *scope_cast<Ty>(ValueType_as_impl<OwnedScopePtr>(v).get());
        }
    }
    // static_assert(dependent_bool<T, false>::value, "A ValueType can not be converted to the given target type");
}

template <bool reference_as_ptr = false, typename T>
decltype(auto) convert_ValueType(T &&t)
{
    static_assert(!requires { typename std::decay_t<T>::no_value_type; });

    if constexpr (InstanceOf<std::decay_t<T>, std::reference_wrapper>) {
        // using Ty = typename std::decay_t<T>::type;
        return convert_ValueType<true>(t.get());
    } else if constexpr (InstanceOf<T, std::optional>) {
        return std::forward<T>(t);
    } else if constexpr (ValueTypePrimitive<std::decay_t<T>> || std::same_as<ValueType, std::decay_t<T>>) {
        return std::forward<T>(t);
    } else if constexpr (String<std::decay_t<T>>) {
        return std::string { std::forward<T>(t) };
    } else if constexpr (std::ranges::range<T>) {
        if constexpr (std::same_as<KeyType_t<std::ranges::range_value_t<T>>, Void>)
            return KeyValueVirtualSequenceRange { std::forward<T>(t) };
        else
            return KeyValueVirtualAssociativeRange { std::forward<T>(t) };
    } else if constexpr (std::is_enum_v<std::decay_t<T>>) {
        if constexpr (std::is_reference_v<T>) {
            return static_cast<std::underlying_type_t<T> &>(t);
        } else {
            return static_cast<std::underlying_type_t<T>>(t);
        }
    } else if constexpr (InstanceOf<std::decay_t<T>, EnumType> || InstanceOf<std::decay_t<T>, BaseEnum>) {
        return EnumHolder { std::forward<T>(t) };
    } else if constexpr (InstanceOf<std::decay_t<T>, Flags>) {
        return FlagsHolder { std::forward<T>(t) };
    } else if constexpr (Execution::Sender<std::decay_t<T>>) {
        return KeyValueSender { std::forward<T>(t) };
    } else if constexpr (InstanceOfA<std::decay_t<T>, TypedBoundApiFunction>) {
        return BoundApiFunction { std::forward<T>(t) };
    } else {
        if constexpr (Pointer<std::decay_t<T>>) {
            return ScopePtr { t };
        } else if constexpr (InstanceOf<std::decay_t<T>, std::unique_ptr>) {
            return ScopePtr { t.get() };
        } else if constexpr (reference_as_ptr) {
            return ScopePtr { &t };
        } else {
            return OwnedScopePtr { std::forward<T>(t) };
        }
    }
    // static_assert(dependent_bool<T, false>::value, "The provided type can not be converted to a ValueType");
}

template <typename T>
requires(ValueTypePrimitive<std::decay_t<T>> || std::same_as<ValueType, std::decay_t<T>>)
    META_EXPORT void to_ValueType_impl(ValueType &v, T &&t);

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