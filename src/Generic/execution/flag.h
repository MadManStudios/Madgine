#pragma once

#include "flagstub.h"

namespace Engine {
namespace Execution {

    template <typename... _Ty>
    struct Flag : FlagStub<_Ty...> {
        using FlagStub<_Ty...>::FlagStub;

        struct CallbackDelay {
            CallbackDelay(ConnectionStack<Connection<FlagStub<_Ty...>, _Ty...>> stack, Flag<_Ty...> *flag)
                : mStack(std::move(stack))
                , mFlag(flag)
            {
            }
            CallbackDelay(CallbackDelay &&) = default;

            ~CallbackDelay()
            {
                while (Connection<FlagStub<_Ty...>, _Ty...> *current = mStack.pop()) {
                    TupleUnpacker::invokeExpand(&Connection<FlagStub<_Ty...>, _Ty...>::set_value, current, *mFlag->mValue);
                }
            }

            ConnectionStack<Connection<FlagStub<_Ty...>, _Ty...>> mStack;
            Flag<_Ty...> *mFlag;
        };

        CallbackDelay emplace(_Ty &&...args)
        {
            ConnectionStack<Connection<FlagStub<_Ty...>, _Ty...>> stack = std::move(this->mStack);
            {
                std::lock_guard guard { this->mStack.mutex() };
                this->mValue.emplace(std::forward<_Ty>(args)...);
            }

            return { std::move(stack), this };
        }
    };
}
}
