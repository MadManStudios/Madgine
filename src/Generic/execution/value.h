#pragma once

#include "valuestub.h"

namespace Engine {
namespace Execution {

    template <typename... _Ty>
    struct Value : ValueStub<_Ty...> {
        using ValueStub<_Ty...>::ValueStub;

        struct CallbackDelay {
            CallbackDelay(ConnectionStack<Connection<ValueStub<_Ty...>, _Ty...>> stack, Value<_Ty...> *value)
                : mStack(std::move(stack))
                , mValue(value)
            {
            }
            CallbackDelay(CallbackDelay &&) = default;

            ~CallbackDelay()
            {
                while (Connection<ValueStub<_Ty...>, _Ty...> *current = mStack.pop()) {
                    TupleUnpacker::invokeExpand(&Connection<ValueStub<_Ty...>, _Ty...>::set_value, current, *mValue->mValue);
                }
            }

            ConnectionStack<Connection<ValueStub<_Ty...>, _Ty...>> mStack;
            Value<_Ty...> *mValue;
        };

        CallbackDelay set(_Ty &&...args)
        {
            ConnectionStack<Connection<ValueStub<_Ty...>, _Ty...>> stack = std::move(this->mStack);
            {
                std::lock_guard guard { this->mStack.mutex() };
                this->mValue = {std::forward<_Ty>(args)...};
            }

            return { std::move(stack), this };
        }

    };
}
}
