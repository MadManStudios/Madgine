#pragma once

#include "signalstub.h"

namespace Engine {
namespace Execution {

    template <typename... _Ty>
    struct Signal : SignalStub<_Ty...> {

        void emit(_Ty... args)
        {
            ConnectionStack<Connection<SignalStub<_Ty...>, _Ty...>> stack = std::move(this->mStack);

            while (Connection<SignalStub<_Ty...>, _Ty...> *current = stack.pop()) {
                current->set_value(args...);
            }
        }
    };
}
}
