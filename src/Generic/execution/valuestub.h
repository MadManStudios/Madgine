#pragma once

#include "algorithm.h"
#include "connection.h"
#include "container/stack.h"
#include "execution.h"
#include "virtualsender.h"

namespace Engine {
namespace Execution {

    template <typename... Ty>
    struct ValueStub : ConnectionSender<ValueStub<Ty...>, Ty...> {
        template <typename... Args>
        ValueStub(Args &&...args)
            : mValue(std::forward<Args>(args)...)
        {
        }

        ValueStub(const ValueStub<Ty...> &other) { }
        ValueStub(ValueStub<Ty...> &&) noexcept { }

        ValueStub<Ty...> &operator=(const ValueStub<Ty...> &other) = delete;

        const std::tuple<Ty...> &operator*() const
        {
            std::lock_guard guard { mStack.mutex() };
            return mValue;
        }

        void enqueue(Connection<ValueStub<Ty...>, Ty...> *con)
        {
            mStack.push(con);
        }

        bool extract(Connection<ValueStub<Ty...>, Ty...> *con)
        {
            return mStack.extract(con);
        }

        template <typename T>
        auto bind(T &&slot)
        {
            TupleUnpacker::invoke(slot, mValue);
            return *this | Execution::then(TupleUnpacker::wrap(std::forward<T>(slot))) | Execution::repeat;
        }

    protected:
        ConnectionStack<Connection<ValueStub<Ty...>, Ty...>> mStack;
        std::tuple<Ty...> mValue;
    };

}
}
