#pragma once

#include "Generic/execution/concepts.h"
#include "Generic/fixed_string.h"

namespace Engine {
namespace Behavior {

    struct Behavior;
    struct BehaviorStateBase;
    struct BehaviorReceiver;
    struct BehaviorError;
    struct BehaviorList;

    struct ParameterTuple;

    struct BehaviorFactoryBase;

    template <fixed_string Name, typename T>
    struct Named;

    struct NamedDescriptor;

    struct HandlerBase;
    struct HandlerManager;

    struct CoroutineBehaviorState;

    template <typename Sender>
    struct BehaviorAwaitableSender;
    template <typename Binding>
    struct BehaviorAwaitableBinding;

    template <Execution::AnySender Sender>
    struct SenderBehaviorState;
}
}
