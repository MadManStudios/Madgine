#pragma once

#include "Generic/callable_view.h"
#include "Madgine/debug/statedescriptor.h"

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

        template <typename T>
        using helper = void(const T &);
        using CB = typename Execution::StateTypes::prepend<Execution::State::Value>::template transform<helper>::template instantiate<CallableView>;
        virtual void visitState(CB visitor) = 0;
    };

}
}
