#include "../metalib.h"

#include "keyvaluepair.h"

#include "argumentlist.h"

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

template <ValueTypePrimitive T>
META_EXPORT ValueType_Return<T> ValueType_as_impl(const ValueType &v)
{
    return v.as<std::decay_t<T>>();
}

#define VALUETYPE_SEP
#define VALUETYPE_IMPL(Type)                                                                                                                    \
    template <>                                                                                                                                 \
    META_EXPORT void to_ValueType_impl(ValueType &v, std::decay_t<Type> &&t) { v = std::move(t); }                                              \
                                                                                                                                                \
    template <>                                                                                                                                 \
    META_EXPORT void to_ValueType_impl<std::decay_t<Type> &>(ValueType & v, std::decay_t<Type> & t) { v = t; }                                  \
                                                                                                                                                \
    template <>                                                                                                                                 \
    META_EXPORT void to_ValueType_impl<const std::decay_t<Type>>(ValueType & v, const std::decay_t<Type> &&t) { v = std::move(t); }             \
                                                                                                                                                \
    template <>                                                                                                                                 \
    META_EXPORT void to_ValueType_impl<const std::decay_t<Type> &>(ValueType & v, const std::decay_t<Type> &t) { v = t; }                       \
                                                                                                                                  \
    template META_EXPORT ValueType_Return<std::decay_t<Type>> ValueType_as_impl<std::decay_t<Type>>(const ValueType &v);

#define VALUETYPE_TYPE(Name, Storage, ...) FOR_EACH(VALUETYPE_IMPL, VALUETYPE_SEP, __VA_ARGS__)

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

}