#pragma once

#include "signal.h"

namespace Engine {
namespace Execution {

	template <typename... Args>
    struct SignalFunctor {
        void operator()(Args... args)
        {
            mSignal.emit(args...);
        }

        SignalStub<Args...> &signal()
        {
            return mSignal;
        }

    private:
        Signal<Args...> mSignal;
    };

}
}