#pragma once

#include "Generic/delayedconstruct.h"
#include "Generic/execution/concepts.h"
#include "Generic/execution/virtualstate.h"
#include "Generic/genericresult.h"
#include "Generic/callable_view.h"
#include "result.h"

namespace Engine {
namespace Reflect {

    struct Receiver : Execution::VirtualReceiverBaseEx<type_pack<Error>, type_pack<const ArgumentList &>, Execution::get_stop_token> {
        template <typename... Args>
        void set_value(Args &&...args)
        {
            static_cast<Execution::VirtualReceiverBase<Error, const ArgumentList &> *>(this)->set_value(ArgumentList { std::forward<Args>(args)... });
        }
    };

    struct SenderStateBase {
        virtual void connect(Receiver &receiver) = 0;
        virtual void start() = 0;
        virtual void stop() = 0;

        template <typename T>
        using helper = void(const T &);
        using CB = typename Execution::StateTypes::template transform<helper>::template instantiate<CallableView>;
        virtual void visitState(CB visitor) = 0;
    };

    template <typename Sender>
    struct SenderState : SenderStateBase {

        using State = Execution::connect_result_t<Sender, Receiver &>;

        SenderState(Sender &&sender)
            : mState(std::forward<Sender>(sender))
        {
        }

        void connect(Receiver &receiver) override
        {
            mState.template emplace<State>(DelayedConstruct {
                [&, sender { std::move(std::get<forward_ref_t<Sender>>(mState)) }]() mutable { return Execution::connect(std::forward<Sender>(sender), receiver); } });
        }

        void start() override
        {
            std::get<State>(mState).start();
        }

        void stop() override
        {
            std::get<State>(mState).stop();
        }

        void visitState(SenderStateBase::CB visitor) override
        {
            Execution::visit_state(&std::get<State>(mState), visitor);
        }

        std::variant<forward_ref_t<Sender>, State> mState;
    };

    struct Sender {

        using is_sender = void;
        using result_type = Error;
        template <template <typename...> typename Tuple>
        using value_types = Tuple<const ArgumentList &>;

        template <Execution::AnySender Inner>
            requires Concepts::DecayedNoneOf<Inner, Sender>
        Sender(Inner &&sender)
            : mState(std::make_shared<SenderState<Inner>>(std::forward<Inner>(sender)))
        {
        }

        Sender() = default;
        Sender(const Sender &) = default;
        Sender(Sender &&) = default;

        Sender &operator=(const Sender &) = default;
        Sender &operator=(Sender &&) = default;

        template <typename Rec>
        struct state : Execution::VirtualState<Receiver, Rec> {
            state(Rec &&rec, std::shared_ptr<SenderStateBase> state)
                : Execution::VirtualState<Receiver, Rec>(std::forward<Rec>(rec))
                , mState(std::move(state))
            {
                mState->connect(*this);
            }

            void start()
            {
                mState->start();
            }

            void stop()
            {
                mState->stop();
            }

            friend void tag_invoke(Execution::visit_state_t, state *s, auto &&visitor)
            {
                if (s) {
                    s->mState->visitState(std::forward<decltype(visitor)>(visitor));
                } else {
                    visitor(Execution::State::Text { "KeyValueSender" });
                }
            }

            std::shared_ptr<SenderStateBase> mState;
        };

        template <typename Rec>
        friend auto tag_invoke(Execution::connect_t, const Sender &sender, Rec &&rec)
        {
            return state<Rec> { std::forward<Rec>(rec), sender.mState };
        }

    private:
        std::shared_ptr<SenderStateBase> mState;
    };

}
}