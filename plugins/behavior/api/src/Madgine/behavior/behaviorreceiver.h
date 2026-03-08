#pragma once

#include "Generic/execution/virtualsender.h"

#include "Meta/keyvalue/argumentlist.h"

#include "Madgine/debug/debuggablesender.h"

#include "named.h"

namespace Engine {
namespace Behavior {

    struct MADGINE_BEHAVIOR_EXPORT BehaviorReceiver : Execution::VirtualReceiverBaseEx<type_pack<KeyValueError>, type_pack<ArgumentList>, Execution::get_stop_token, Debug::get_debug_context, Log::get_log, get_named_d> {
        template <typename... Args>
        void set_value(Args &&...args)
        {
            static_cast<Execution::VirtualReceiverBase<KeyValueError, ArgumentList> *>(this)->set_value(ArgumentList { std::forward<Args>(args)... });
        }
    };

    template <typename Rec, typename Base = BehaviorReceiver>
    struct VirtualBehaviorState : Execution::VirtualState<Base, Rec> {

        using Execution::VirtualState<Base, Rec>::VirtualState;
    };

    template <typename F, typename... Args>
    auto make_simple_behavior_sender(F &&f, Args &&...args)
    {
        return Execution::make_sender<KeyValueError, ArgumentList>(
            [args = std::tuple<Args...> { std::forward<Args>(args)... }, f { forward_capture<F>(f) }]<typename Rec>(Rec &&rec) mutable {
                return TupleUnpacker::constructExpand<VirtualBehaviorState<Rec, Execution::SimpleInheritedState<F, std::tuple<Args...>, BehaviorReceiver>>>(std::forward<Rec>(rec), std::forward<F>(f), std::move(args));
            });
    }

}
}