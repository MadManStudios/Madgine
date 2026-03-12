#pragma once

#include "signalstub.h"

namespace Engine {
namespace Execution {

    template <typename R, typename... _Ty>
    struct Signal : SignalStub<R, _Ty...> {

        void emit(_Ty... args)
        {
            ConnectionStack<Connection<SignalStub<R, _Ty...>>> stack = std::move(this->mStack);

            while (Connection<SignalStub<R, _Ty...>> *current = stack.pop()) {
                current->set_value(args...);
            }
        }
    };
}
}
