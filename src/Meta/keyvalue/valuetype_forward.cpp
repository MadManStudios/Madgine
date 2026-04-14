#include "../metalib.h"

#include "argumentlist.h"
#include "keyvaluepair.h"

namespace Engine {

ValueType &KeyValuePair_key(KeyValuePair &p)
{
    return p.mKey;
}

ValueType &KeyValuePair_value(KeyValuePair &p)
{
    return p.mValue;
}

const ValueType &getArgument(const ArgumentList &args, size_t index)
{
    return args.at(index);
}

bool ValueType_isNull(const ValueType &v)
{
    return v.is<std::monostate>();
}

ValueTypeDesc ValueType_type(const ValueType &v)
{
    return v.type();
}

template <ValueTypeStorage T>
META_EXPORT const T &ValueType_as(const ValueType &v)
{
    return v.as<T>();
}

template <ValueTypeStorage T>
META_EXPORT T &ValueType_as(ValueType &v)
{
    return v.as<T>();
}

#define VALUETYPE_SEP
#define VALUETYPE_IMPL(Type)                                                                                                        \
    template <>                                                                                                                     \
    META_EXPORT void to_ValueType_impl(ValueType &v, std::decay_t<Type> &&t) { v = std::move(t); }                                  \
                                                                                                                                    \
    template <>                                                                                                                     \
    META_EXPORT void to_ValueType_impl<std::decay_t<Type> &>(ValueType & v, std::decay_t<Type> & t) { v = t; }                      \
                                                                                                                                    \
    template <>                                                                                                                     \
    META_EXPORT void to_ValueType_impl<const std::decay_t<Type>>(ValueType & v, const std::decay_t<Type> &&t) { v = std::move(t); } \
                                                                                                                                    \
    template <>                                                                                                                     \
    META_EXPORT void to_ValueType_impl<const std::decay_t<Type> &>(ValueType & v, const std::decay_t<Type> &t) { v = t; }

#define VALUETYPE_TYPE(Name, Storage, ...)                                         \
    FOR_EACH(VALUETYPE_IMPL, VALUETYPE_SEP, __VA_ARGS__)                           \
    template META_EXPORT const Storage &ValueType_as<Storage>(const ValueType &v); \
    template META_EXPORT Storage &ValueType_as<Storage>(ValueType & v);

#include "valuetypedefinclude.h"
#undef VALUETYPE_IMPL

template <>
META_EXPORT void to_ValueType_impl<ValueType>(ValueType &v, ValueType &&t)
{
    v = std::move(t);
}

template <>
META_EXPORT void to_ValueType_impl<ValueType &>(ValueType &v, ValueType &t)
{
    v = t;
}

template <>
META_EXPORT void to_ValueType_impl<const ValueType>(ValueType &v, const ValueType &&t)
{
    v = std::move(t);
}

template <>
META_EXPORT void to_ValueType_impl<const ValueType &>(ValueType &v, const ValueType &t)
{
    v = t;
}

void ValueType_erased(CallableView<void(ValueType &)> cb)
{
    ValueType v;
    cb(v);
}

}