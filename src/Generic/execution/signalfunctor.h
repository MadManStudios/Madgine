#pragma once

#include "signal.h"

namespace Engine {
namespace Execution {

    template <typename R, typename... Args>
    struct SignalFunctor {
        void operator()(Args... args)
        {
            mSignal.emit(args...);
        }

        SignalStub<R, Args...> &signal()
        {
            return mSignal;
        }

    private:
        Signal<R, Args...> mSignal;
    };

}
}