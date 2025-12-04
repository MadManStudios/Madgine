#pragma once

#include "Generic/callable_view.h"
#include "Generic/execution/statedescriptor.h"

namespace Engine {
namespace Behavior {

    struct BehaviorStateBase {
        virtual ~BehaviorStateBase() = default;

        virtual void connect(BehaviorReceiver &rec) = 0;
        virtual void start() = 0;
        virtual void stop() = 0;
        virtual void destroy()
        {
            delete this;
        }

        virtual void visitState(CallableView<void(const Execution::StateDescriptor &)> visitor) = 0;
    };

}
}
