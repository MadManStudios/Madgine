#pragma once

#include "util.h"

namespace Engine {
namespace Reflect {

    struct META_EXPORT Function {
        template <typename R, typename... Args, size_t... I>
        static Result unpackHelper(void (*f)(), Value &retVal, const ArgumentList &args, std::index_sequence<I...>)
        {
            return invoke(retVal, reinterpret_cast<R (*)(Args...)>(f), getArgument(args, I)...);
        }

        template <typename R, typename... Args>
        static Result unpackApiMethod(void (*f)(), Value &retVal, const ArgumentList &args)
        {
            return unpackHelper<R, Args...>(f, retVal, args, std::index_sequence_for<Args...>());
        }

        template <typename R, typename... Args>
        Function(R (*f)(Args...))
            : mFunction(reinterpret_cast<void (*)()>(f))
            , mWrapper(&unpackApiMethod<R, Args...>)
        {
        }

        bool operator==(const Function &other) const
        {
            return mFunction == other.mFunction;
        }

        Result operator()(Value &retVal, const ArgumentList &args) const;

    private:
        void (*mFunction)();
        Result (*mWrapper)(void (*)(), Value &, const ArgumentList &);
    };

}
}