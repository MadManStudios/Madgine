#pragma once

#include "algorithm.h"
#include "connection.h"
#include "execution.h"
#include "virtualsender.h"
#include "container/stack.h"

namespace Engine {
namespace Execution {

    template <typename... Ty>
    struct FlagStub : ConnectionSender<FlagStub<Ty...>, Ty...> {
        template <typename... Args>
        FlagStub(Args &&...args)
            : mValue(std::forward<Args>(args)...)
        {
        }

        FlagStub(const FlagStub<Ty...> &other) { }
        FlagStub(FlagStub<Ty...> &&) noexcept { }

        FlagStub<Ty...> &operator=(const FlagStub<Ty...> &other) = delete;

        bool isSet() const
        {
            std::lock_guard guard { mStack.mutex() };
            return static_cast<bool>(mValue);
        }

        const std::tuple<Ty...> &operator*() const
        {
            std::lock_guard guard { mStack.mutex() };
            return *mValue;
        }

        void enqueue(Connection<FlagStub<Ty...>, Ty...> *con)
        {
            if (mValue) {
                TupleUnpacker::invokeExpand(&Connection<FlagStub<Ty...>, Ty...>::set_value, con, *mValue);
            } else {
                mStack.push(con);
            }
        }

        bool extract(Connection<FlagStub<Ty...>, Ty...> *con)
        {
            return mStack.extract(con);
        }

    protected:
        ConnectionStack<Connection<FlagStub<Ty...>, Ty...>> mStack;
        std::optional<std::tuple<Ty...>> mValue;        
    };

}
}
