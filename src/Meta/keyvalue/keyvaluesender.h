#pragma once

#include "Generic/delayedconstruct.h"
#include "Generic/execution/concepts.h"
#include "Generic/execution/virtualstate.h"
#include "Generic/genericresult.h"

namespace Engine {

using KeyValueReceiver = Execution::VirtualReceiverBaseEx<type_pack<GenericResult>, type_pack<const ArgumentList &>, Execution::get_stop_token>;

struct KeyValueSenderStateBase {
    virtual void connect(KeyValueReceiver &receiver) = 0;
    virtual void start() = 0;
};

template <typename Sender>
struct KeyValueSenderState : KeyValueSenderStateBase {

    using State = Execution::connect_result_t<Sender, KeyValueReceiver &>;

    KeyValueSenderState(Sender &&sender)
        : mState(std::forward<Sender>(sender))
    {
    }

    virtual void connect(KeyValueReceiver &receiver) override
    {
        mState.template emplace<State>(DelayedConstruct<State> {
            [&, sender { std::forward<Sender>(std::get<Sender>(mState)) }]() mutable { return Execution::connect(std::move(sender), receiver); } });
    }

    virtual void start() override
    {
        std::get<State>(mState).start();
    }

    std::variant<Sender, State> mState;
};

struct KeyValueSender {

    using is_sender = void;
    using result_type = GenericResult;
    template <template <typename...> typename Tuple>
    using value_types = Tuple<>;

    template <Execution::Sender Sender>
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
    struct state : Execution::VirtualState<Rec, KeyValueReceiver> {
        state(Rec &&rec, std::shared_ptr<KeyValueSenderStateBase> state)
            : Execution::VirtualState<Rec, KeyValueReceiver>(std::forward<Rec>(rec))
            , mState(std::move(state))
        {
            mState->connect(*this);
        }

        void start()
        {
            mState->start();
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