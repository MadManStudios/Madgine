#pragma once

#include "flagstub.h"

namespace Engine {
namespace Execution {

    template <typename R, typename... _Ty>
    struct Flag : FlagStub<R, _Ty...> {
        using FlagStub<R, _Ty...>::FlagStub;

        struct CallbackDelay {
            CallbackDelay(ConnectionStack<Connection<FlagStub<R, _Ty...>>> stack, Flag<R, _Ty...> *flag)
                : mStack(std::move(stack))
                , mFlag(flag)
            {
            }
            CallbackDelay(CallbackDelay &&) = default;

            ~CallbackDelay()
            {
                while (Connection<FlagStub<R, _Ty...>> *current = mStack.pop()) {
                    mFlag->mStorage.reproduce(*current);
                }
            }

            ConnectionStack<Connection<FlagStub<R, _Ty...>>> mStack;
            Flag<R, _Ty...> *mFlag;
        };

        template <typename... Ty>
        CallbackDelay set_value(Ty &&...args)
        {
            ConnectionStack<Connection<FlagStub<R, _Ty...>>> stack = std::move(this->mStack);
            {
                std::lock_guard guard { this->mStack.mutex() };
                this->mStorage.set_value(std::forward<Ty>(args)...);
            }

            return { std::move(stack), this };
        }

        CallbackDelay set_done()
        {
            ConnectionStack<Connection<FlagStub<R, _Ty...>>> stack = std::move(this->mStack);
            {
                std::lock_guard guard { this->mStack.mutex() };
                this->mStorage.set_done();
            }

            return { std::move(stack), this };
        }

        CallbackDelay set_error(patch_void_t<R> &&r)
        {
            ConnectionStack<Connection<FlagStub<R, _Ty...>>> stack = std::move(this->mStack);
            {
                std::lock_guard guard { this->mStack.mutex() };
                this->mStorage.set_error(std::forward<R>(r));
            }

            return { std::move(stack), this };
        }

        void reset()
        {
            std::lock_guard guard { this->mStack.mutex() };
            this->mStorage.reset();
        }
    };
}
}
