#pragma once

#include "Generic/any.h"
#include "Generic/delayedconstruct.h"
#include "Generic/functor.h"

#include "Interfaces/debug/stacktrace.h"

#include "Madgine/debug/debuggablesender.h"

#include "behaviorcoroutine.h"
#include "behaviorerror.h"
#include "behaviorreceiver.h"

namespace Engine {
namespace Behavior {

    struct MADGINE_BEHAVIOR_EXPORT Behavior {

        static void destroyState(BehaviorStateBase *state);
        using StatePtr = std::unique_ptr<BehaviorStateBase, Functor<&destroyState>>;

        Behavior() = default;
        Behavior(StatePtr state);

        template <Execution::Sender Sender>
        Behavior(Sender &&sender)
            : mState(new SenderBehaviorState<Sender>(std::forward<Sender>(sender)))
        {
        }

        Behavior &operator=(StatePtr state);

        StatePtr release()
        {
            return std::move(mState);
        }

        StatePtr connect(BehaviorReceiver &receiver);

        struct MADGINE_BEHAVIOR_EXPORT state : BehaviorReceiver {

            state(StatePtr state);

            void start();
            void stop();

            friend MADGINE_BEHAVIOR_EXPORT void tag_invoke(Execution::visit_state_t, state *state, CallableView<void(const Execution::StateDescriptor &)> visitor);

        protected:
            void connect();

            StatePtr mState;
        };

        template <typename Rec>
        struct state_helper : VirtualBehaviorState<Rec, state> {
            state_helper(Rec &&rec, StatePtr statePtr)
                : VirtualBehaviorState<Rec, state>(std::forward<Rec>(rec), std::move(statePtr))
            {
                this->connect();
            }
        };

        using is_sender = void;

        using result_type = BehaviorError;
        template <template <typename...> typename Tuple>
        using value_types = Tuple<ArgumentList>;

        template <typename Rec, std::same_as<Behavior> T> // Necessary to prevent implicit conversion
        friend auto tag_invoke(Execution::connect_t, T &&behavior, Rec &&rec)
        {
            assert(behavior.mState);
            return state_helper<Rec> { std::forward<Rec>(rec), std::move(behavior.mState) };
        }

        using promise_type = CoroutineBehaviorState;

        StatePtr mState;
    };

    template <typename Sender>
    struct BehaviorAwaitableSender;
    template <typename Binding>
    struct BehaviorAwaitableBinding;

    template <Execution::Sender Sender>
    struct SenderBehaviorState : BehaviorStateBase {

        using State = Execution::connect_result_t<Sender, BehaviorReceiver &>;

        SenderBehaviorState(Sender &&sender)
            : mData(std::forward<Sender>(sender))
        {
        }

        void connect(BehaviorReceiver &rec) override
        {
            Sender sender = std::forward<Sender>(std::get<Sender>(mData));
            mData.template emplace<State>(
                DelayedConstruct<State> { [&]() { return Execution::connect(std::forward<Sender>(sender), rec); } });
        }

        void start() override
        {
            std::get<State>(mData).start();
        }

        void stop() override
        {
            std::get<State>(mData).stop();
        }

        void visitState(CallableView<void(const Execution::StateDescriptor &)> visitor) override
        {
            Execution::visit_state(std::holds_alternative<State>(mData) ? &std::get<State>(mData) : nullptr, visitor);
        }

        std::variant<Sender, State> mData;
    };

}
}