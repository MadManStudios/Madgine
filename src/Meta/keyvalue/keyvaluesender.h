#pragma once

#include "Generic/delayedconstruct.h"
#include "Generic/execution/concepts.h"
#include "Generic/execution/virtualstate.h"
#include "Generic/genericresult.h"
#include "Generic/callable_view.h"
#include "keyvalueresult.h"

namespace Engine {

using KeyValueReceiver = Execution::VirtualReceiverBaseEx<type_pack<KeyValueError>, type_pack<const ArgumentList &>, Execution::get_stop_token>;

struct KeyValueSenderStateBase {
    virtual void connect(KeyValueReceiver &receiver) = 0;
    virtual void start() = 0;

    virtual void visitState(CallableView<void(const Execution::StateDescriptor &)> visitor) = 0;
};

template <typename Sender>
struct KeyValueSenderState : KeyValueSenderStateBase {

    using State = Execution::connect_result_t<Sender, KeyValueReceiver &>;

    KeyValueSenderState(Sender &&sender)
        : mState(std::forward<Sender>(sender))
    {
    }

    void connect(KeyValueReceiver &receiver) override
    {
        mState.template emplace<State>(DelayedConstruct<State> {
            [&, sender { std::forward<Sender>(std::get<Sender>(mState)) }]() mutable { return Execution::connect(std::move(sender), receiver); } });
    }

    void start() override
    {
        std::get<State>(mState).start();
    }

    void visitState(CallableView<void(const Execution::StateDescriptor &)> visitor) override
    {
        Execution::visit_state(&std::get<State>(mState), visitor);
    }

    std::variant<Sender, State> mState;
};

struct KeyValueSender {

    using is_sender = void;
    using result_type = KeyValueError;
    template <template <typename...> typename Tuple>
    using value_types = Tuple<>;

    template <Execution::AnySender Sender>
        requires DecayedNoneOf<Sender, KeyValueSender>
    KeyValueSender(Sender &&sender)
        : mState(std::make_shared<KeyValueSenderState<Sender>>(std::forward<Sender>(sender)))
    {
    }

    KeyValueSender() = default;
    KeyValueSender(const KeyValueSender &) = default;
    KeyValueSender(KeyValueSender &&) = default;

    KeyValueSender &operator=(const KeyValueSender &) = default;
    KeyValueSender &operator=(KeyValueSender &&) = default;

    template <typename Rec>
    struct state : Execution::VirtualState<KeyValueReceiver, Rec> {
        state(Rec &&rec, std::shared_ptr<KeyValueSenderStateBase> state)
            : Execution::VirtualState<KeyValueReceiver, Rec>(std::forward<Rec>(rec))
            , mState(std::move(state))
        {
            mState->connect(*this);
        }

        void start()
        {
            mState->start();
        }

        friend void tag_invoke(Execution::visit_state_t, state *s, auto &&visitor)
        {
            if (s) {
                s->mState->visitState(std::forward<decltype(visitor)>(visitor));
            } else {
                visitor(Execution::State::Text { "KeyValueSender" });
            }
        }

        std::shared_ptr<KeyValueSenderStateBase> mState;
    };

    template <typename Rec>
    friend auto tag_invoke(Execution::connect_t, const KeyValueSender &sender, Rec &&rec)
    {
        return state<Rec> { std::forward<Rec>(rec), sender.mState };
    }

private:
    std::shared_ptr<KeyValueSenderStateBase> mState;
};

}