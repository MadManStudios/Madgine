#pragma once

#include "algorithm.h"
#include "connection.h"
#include "container/stack.h"

namespace Engine {
namespace Execution {

    template <typename R, typename... Ty>
    struct SignalStub : ConnectionSender<SignalStub<R, Ty...>, R, Ty...> {
        SignalStub() = default;

        SignalStub(const SignalStub<R, Ty...> &other)
        {
        }

        SignalStub(SignalStub<R, Ty...> &&) noexcept
        {
        }

        SignalStub<R, Ty...> &operator=(const SignalStub<R, Ty...> &other) = delete;

        template <typename T, typename... Args>
        auto connect(void (T::*f)(Args...), T *t)
        {
            return connect([t, f](Args... args) { return (t->*f)(std::forward<Args>(args)...); });
        }

        template <typename T>
        auto connect(T &&slot)
        {
            return *this | Execution::then(TupleUnpacker::wrap(std::forward<T>(slot))) | Execution::repeat;
        }

        void enqueue(Connection<SignalStub<R, Ty...>> *con)
        {
            mStack.push(con);
        }

        bool extract(Connection<SignalStub<R, Ty...>> *con)
        {
            return mStack.extract(con);
        }

    protected:
        ConnectionStack<Connection<SignalStub<R, Ty...>>> mStack;
    };

}
}
