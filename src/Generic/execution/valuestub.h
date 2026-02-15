#pragma once

#include "algorithm.h"
#include "connection.h"
#include "container/stack.h"
#include "virtualsender.h"

namespace Engine {
namespace Execution {

    template <typename...>
    struct ValueStub;

    template <typename... Ty>
    struct ValueConnection : Connection<ValueStub<Ty...>, Ty...> {
        ValueConnection(ValueStub<Ty...> &stub, std::tuple<Ty...> value)
            : Connection<ValueStub<Ty...>, Ty...>(stub)
            , mValue(std::move(value))
        {
        }

        void start()
        {
            this->mStub.enqueue(this);
        }

        std::tuple<Ty...> mValue;
    };

    template <typename... Ty>
    struct ValueSender {
        using result_type = void;
        template <template <typename...> typename Tuple>
        using value_types = Tuple<Ty...>;

        using is_sender = void;

        template <typename Rec>
        friend auto tag_invoke(Execution::connect_t, ValueSender<Ty...> &&sender, Rec &&rec)
        {
            return Execution::VirtualState<ValueConnection<Ty...>, Rec>(std::forward<Rec>(rec), sender.mStub, std::move(sender.mValue));
        }

        ValueStub<Ty...> &mStub;
        std::tuple<Ty...> mValue;
    };

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
            std::unique_lock guard { mStack.mutex() };
            return mValue;
        }

        void enqueue(Connection<ValueStub<Ty...>, Ty...> *con)
        {
            mStack.push(con);
        }

        void enqueue(ValueConnection<Ty...> *con)
        {
            std::unique_lock guard { mStack.mutex() };
            if (con->mValue != mValue) {
                std::tuple<Ty...> value = mValue;
                guard.unlock();
                TupleUnpacker::invokeExpand(&Connection<ValueStub<Ty...>, Ty...>::set_value, con, std::move(value));                
            } else {
                mStack.push(con, guard);
            }            
        }

        bool extract(Connection<ValueStub<Ty...>, Ty...> *con)
        {
            return mStack.extract(con);
        }

        template <typename T>
        auto bind(T &&slot)
        {
            TupleUnpacker::invokeFromTuple(slot, mValue);
            return *this | Execution::then(std::forward<T>(slot)) | Execution::repeat;
        }

        ValueSender<Ty...> sender(Ty &&...args)
        {
            return ValueSender<Ty...> { *this, { std::forward<Ty>(args)... } };
        }

    protected:
        ConnectionStack<Connection<ValueStub<Ty...>, Ty...>> mStack;
        std::tuple<Ty...> mValue;
    };

}
}
