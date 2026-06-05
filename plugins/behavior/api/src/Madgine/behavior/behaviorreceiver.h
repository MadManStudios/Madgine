#pragma once

#include "Generic/execution/virtualstate.h"

#include "Meta/reflect/argumentlist.h"

#include "Madgine/debug/debuggablesender.h"

#include "named.h"

namespace Engine {
namespace Behavior {

    struct MADGINE_BEHAVIOR_EXPORT BehaviorReceiver : Execution::VirtualReceiverBaseEx<type_pack<Reflect::Error>, type_pack<Reflect::ArgumentList>, Execution::get_stop_token, Debug::get_debug_context, Platform::Log::get_log, get_named_d> {
        template <typename... Args>
        void set_value(Args &&...args)
        {
            static_cast<Execution::VirtualReceiverBase<Reflect::Error, Reflect::ArgumentList> *>(this)->set_value(Reflect::ArgumentList { std::forward<Args>(args)... });
        }
    };

    template <typename Rec, typename Base = BehaviorReceiver>
    struct VirtualBehaviorState : Execution::VirtualState<Base, Rec> {

        using Execution::VirtualState<Base, Rec>::VirtualState;
    };

}
}