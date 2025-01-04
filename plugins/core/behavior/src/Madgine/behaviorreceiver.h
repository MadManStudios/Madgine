#pragma once

#include "Generic/execution/virtualsender.h"
#include "Meta/keyvalue/argumentlist.h"
#include "behaviorerror.h"

#include "Meta/keyvalue/valuetype_forward.h"

#include "bindings.h"

#include "Madgine/debug/debuggablesender.h"

namespace Engine {

struct MADGINE_BEHAVIOR_EXPORT BehaviorReceiver : Execution::VirtualReceiverBaseEx<type_pack<BehaviorError>, type_pack<ArgumentList>, Execution::get_stop_token, Execution::get_debug_location, Log::get_log> {
    template <typename... Args>
    void set_value(Args &&...args)
    {
        static_cast<Execution::VirtualReceiverBase<BehaviorError, ArgumentList> *>(this)->set_value(ArgumentList { std::forward<Args>(args)... });
    }

    virtual BehaviorError getBinding(std::string_view name, ValueType &out) = 0;

    BehaviorError getBindingHelper(std::string_view name, CallableView<void(const ValueType &)> cb);

    template <typename O>
    friend auto tag_invoke(get_binding_d_t, BehaviorReceiver &rec, std::string_view name, O &out)
    {
        if constexpr (std::same_as<O, ValueType> || std::same_as<O, ValueTypeRef>) {
            return rec.getBinding(name, out);
        } else {
            auto f = [&](const ValueType &v) {
                out = ValueType_as<O>(v);
            };
            return rec.getBindingHelper(name, CallableView<void(const ValueType &)> { f });
        }
    }

};

template <typename Rec, typename Base = BehaviorReceiver>
struct VirtualBehaviorState : Execution::VirtualState<Rec, Base> {

    using Execution::VirtualState<Rec, Base>::VirtualState;

    BehaviorError getBinding(std::string_view name, ValueType &out) override
    {
        ValueTypeRef outRef { out };
        return get_binding_d(this->mRec, name, outRef);
    }
};

template <typename F, typename... Args>
auto make_simple_behavior_sender(F &&f, Args &&...args)
{
    return Execution::make_sender<BehaviorError, ArgumentList>(
        [args = std::tuple<Args...> { std::forward<Args>(args)... }, f { forward_capture<F>(std::forward<F>(f)) }]<typename Rec>(Rec &&rec) mutable {
            return TupleUnpacker::constructExpand<VirtualBehaviorState<Rec, Execution::SimpleState<F, std::tuple<Args...>, BehaviorReceiver>>>(std::forward<Rec>(rec), std::forward<F>(f), std::move(args));
        });
}

}