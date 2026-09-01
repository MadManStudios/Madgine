#pragma once

#include "behavior.h"
#include "behaviorhandle.h"
#include "behaviorreceiver.h"
#include "parametertuple.h"

namespace Engine {
namespace Behavior {

    struct MADGINE_BEHAVIOR_EXPORT BehaviorSender {        

        BehaviorSender(BehaviorHandle handle);

        Behavior create() const;

        BehaviorHandle mHandle;
        ParameterTuple mParameters;
    };

}
}