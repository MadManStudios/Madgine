#include "../metalib.h"

#include "argumentlist.h"
#include "value.h"

namespace Engine {
namespace Reflect {

    const Value &getArgument(const ArgumentList &args, size_t index)
    {
        return args.at(index);
    }

    size_t argumentCount(const ArgumentList &args)
    {
        return args.size();
    }

    bool Value_isNull(const Value &v)
    {
        return v.is<std::monostate>();
    }

    Type Value_type(const Value &v)
    {
        return v.type();
    }

    template <ValueStorage T>
    META_EXPORT const T &Value_as(const Value &v)
    {
        return v.as<T>();
    }

    template <ValueStorage T>
    META_EXPORT T &Value_as(Value &v)
    {
        return v.as<T>();
    }

#define VALUE_SEP
#define VALUE_IMPL(Type)                                                                                                        \
    template <>                                                                                                                     \
    META_EXPORT void toValue_impl(Value &v, std::decay_t<Type> &&t) { v = std::move(t); }                                  \
                                                                                                                                    \
    template <>                                                                                                                     \
    META_EXPORT void toValue_impl<std::decay_t<Type> &>(Value & v, std::decay_t<Type> & t) { v = t; }                      \
                                                                                                                                    \
    template <>                                                                                                                     \
    META_EXPORT void toValue_impl<const std::decay_t<Type>>(Value & v, const std::decay_t<Type> &&t) { v = std::move(t); } \
                                                                                                                                    \
    template <>                                                                                                                     \
    META_EXPORT void toValue_impl<const std::decay_t<Type> &>(Value & v, const std::decay_t<Type> &t) { v = t; }

#define VALUE_TYPE(Name, Storage, ...)                                         \
    FOR_EACH(VALUE_IMPL, VALUE_SEP, __VA_ARGS__)                           \
    template META_EXPORT const Storage &Value_as<Storage>(const Value &v); \
    template META_EXPORT Storage &Value_as<Storage>(Value & v);

#include "valuedefinclude.h"
#undef VALUETYPE_IMPL

    template <>
    META_EXPORT void toValue_impl<Value>(Value &v, Value &&t)
    {
        v = std::move(t);
    }

    template <>
    META_EXPORT void toValue_impl<Value &>(Value &v, Value &t)
    {
        v = t;
    }

    template <>
    META_EXPORT void toValue_impl<const Value>(Value &v, const Value &&t)
    {
        v = std::move(t);
    }

    template <>
    META_EXPORT void toValue_impl<const Value &>(Value &v, const Value &t)
    {
        v = t;
    }

    void Value_erased(CallableView<void(Value &)> cb)
    {
        Value v;
        cb(v);
    }

}
}