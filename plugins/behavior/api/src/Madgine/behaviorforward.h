#pragma once

#include "Generic/execution/concepts.h"
#include "Generic/fixed_string.h"

namespace Engine {
namespace Behavior {

    struct Behavior;
    struct BehaviorStateBase;
    struct BehaviorReceiver;
    struct BehaviorList;

    struct ParameterTuple;
    struct BehaviorDescriptor;

    struct BehaviorFactoryBase;

    template <fixed_string Name, typename T>
    struct Named;

    struct HandlerBase;
    struct HandlerManager;

    struct CoroutineBehaviorState;

    template <typename Binding>
    struct BehaviorAwaitableBinding;

    template <Execution::AnySender Sender>
    struct SenderBehaviorState;

    template <typename T>
    struct Bindable;

}
}
