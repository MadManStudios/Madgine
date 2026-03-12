#pragma once

#include "connection.h"
#include "container/stack.h"
#include "storage.h"

namespace Engine {
namespace Execution {

    template <typename R, typename... Ty>
    struct FlagStub : ConnectionSender<FlagStub<R, Ty...>, R, Ty...> {
        FlagStub() = default;
        FlagStub(const FlagStub<R, Ty...> &other) { }
        FlagStub(FlagStub<R, Ty...> &&) noexcept { }

        FlagStub<R, Ty...> &operator=(const FlagStub<R, Ty...> &other) = delete;

        bool isSet() const
        {
            std::unique_lock guard { mStack.mutex() };
            return !mStorage.is_null();
        }

        bool isValue() const
        {
            std::unique_lock guard { mStack.mutex() };
            return mStorage.is_value();
        }

        decltype(auto) operator*() const
        {
            std::unique_lock guard { mStack.mutex() };
            return mStorage.value().get();
        }

        void enqueue(Connection<FlagStub<R, Ty...>> *con)
        {
            std::unique_lock guard { mStack.mutex() };
            if (!mStorage.is_null()) {
                guard.unlock();
                mStorage.reproduce(*con);                
            } else {
                mStack.push(con, guard);
            }
        }

        bool extract(Connection<FlagStub<R, Ty...>> *con)
        {
            return mStack.extract(con);
        }

    protected:
        ConnectionStack<Connection<FlagStub<R, Ty...>>> mStack;
        ResultStorageImpl<ValueStorageImpl<Ty...>, ErrorStorageImpl<R>> mStorage;
    };

}
}
