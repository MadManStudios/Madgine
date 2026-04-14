#pragma once

#include "valuetype_forward.h"

namespace Engine {

struct META_EXPORT KeyValueFunction {
    template <typename R, typename... Args, size_t... I>
    static KeyValueResult unpackHelper(void (*f)(), ValueType &retVal, const ArgumentList &args, std::index_sequence<I...>)
    {
        return ValueType_unwrap(retVal, reinterpret_cast<R (*)(Args...)>(f), getArgument(args, I)...);
    }

    template <typename R, typename... Args>
    static KeyValueResult unpackApiMethod(void (*f)(), ValueType &retVal, const ArgumentList &args)
    {
        return unpackHelper<R, Args...>(f, retVal, args, std::index_sequence_for<Args...>());
    }

    template <typename R, typename... Args>
    KeyValueFunction(R (*f)(Args...))
        : mFunction(reinterpret_cast<void (*)()>(f))
        , mWrapper(&unpackApiMethod<R, Args...>)
    {
    }

    bool operator==(const KeyValueFunction &other) const
    {
        return mFunction == other.mFunction;
    }

    KeyValueResult operator()(ValueType &retVal, const ArgumentList &args) const;

private:
    void (*mFunction)();
    KeyValueResult (*mWrapper)(void (*)(), ValueType &, const ArgumentList &);
};

}