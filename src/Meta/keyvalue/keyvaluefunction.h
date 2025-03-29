#pragma once

#include "valuetype_forward.h"

namespace Engine {

struct META_EXPORT KeyValueFunction {
    template <typename R, typename... Args, size_t... I>
    static void unpackHelper(void (*f)(), ValueType &retVal, const ArgumentList &args, std::index_sequence<I...>)
    {
        to_ValueType(retVal, invoke_patch_void<std::monostate>(reinterpret_cast<R (*)(Args...)>(f), ValueType_as<std::remove_cv_t<std::remove_reference_t<Args>>>(getArgument(args, I))...));
    }

    template <typename R, typename... Args>
    static void unpackApiMethod(void (*f)(), ValueType &retVal, const ArgumentList &args)
    {
        unpackHelper<R, Args...>(f, retVal, args, std::index_sequence_for<Args...>());
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

    void operator()(ValueType &retVal, const ArgumentList &args) const;

private:
    void (*mFunction)();
    void (*mWrapper)(void (*)(), ValueType &, const ArgumentList &);
};

}