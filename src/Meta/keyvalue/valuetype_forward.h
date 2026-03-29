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
META_EXPORT ValueType_Return<T> ValueType_as_impl(const ValueType &v);

template <typename T>
T variantHelper(const ValueType &v)
{
    throw 0;
}

template <typename T, typename Ty, typename... Tys>
T variantHelper(const ValueType &v)
{
    if (ValueType_is<Ty>(v)) {
        return Ty { ValueType_as<Ty>(v) };
    } else {
        return variantHelper<T, Tys...>(v);
    }
}

template <typename Ty>
decltype(auto) ValueType_as(const ValueType &v)
{
    using T = decayed_t<Ty>;

    if constexpr (InstanceOf<T, std::optional>) {
        if (ValueType_isNull(v))
            return T {};
        else {
            return T { ValueType_as<typename is_instance<T, std::optional>::argument_types::template unpack_unique<>>(v) };
        }
    } else if constexpr (InstanceOf<T, std::variant>) {
        return [&]<typename... U>(type_pack<U...>) -> T {
            int count = (ValueType_is<U>(v) + ...);
            assert(count == 1);
            return variantHelper<T, U...>(v);
        }(typename is_instance<T, std::variant>::argument_types {});
    } else if constexpr (std::same_as<T, ValueType>) {
        return v;
    } else if constexpr (ValueTypePrimitive<T>) {
        return ValueType_as_impl<T>(v);
    } else if constexpr (std::ranges::range<T>) {
        if constexpr (std::same_as<KeyType_t<typename T::iterator::value_type>, Void>)
            return ValueType_as_impl<KeyValueVirtualSequenceRange>(v).safe_cast<T>();
        else
            return ValueType_as_impl<KeyValueVirtualAssociativeRange>(v).safe_cast<T>();
    } else if constexpr (InstanceOf<std::decay_t<T>, EnumImpl>) {
        return ValueType_as_impl<EnumHolder>(v).safe_cast<T>();
    } else if constexpr (InstanceOf<std::decay_t<T>, Flags>) {
        return ValueType_as_impl<FlagsHolder>(v).safe_cast<T>();
    } else if constexpr (Execution::AnyBinding<T>) {
        return (ValueType_as_impl<KeyValueBinding>(v)->*&ValueType_as<typename T::type>)();
    } else {
        using U = resolveCustomScopePtr_t<std::remove_reference_t<T>, true>;
        std::remove_pointer_t<U> *ptr = scope_cast<std::remove_pointer_t<U>>(ValueType_as_impl<ScopePtr>(v));
        if constexpr (Pointer<U>) {
            return ptr;
        } else {
            return *ptr;
        }
    }
    // static_assert(dependent_bool<T, false>::value, "A ValueType can not be converted to the given target type");
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