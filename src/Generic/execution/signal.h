#pragma once

#include "signalstub.h"

namespace Engine {
namespace Execution {

    template <typename R, typename... Ty>
    struct Signal : SignalStub<R, Ty...> {

        using SignalStub<R, Ty...>::SignalStub;

        void emit(Ty... args)
        {
            ConnectionStack<Connection<SignalStub<R, Ty...>>> stack = std::move(this->mStack);

            while (Connection<SignalStub<R, Ty...>> *current = stack.pop()) {
                current->set_value(args...);
            }
        }
    };
}
}
