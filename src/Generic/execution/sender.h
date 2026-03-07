#pragma once

#include "../callable_view.h"
#include "../delayedconstruct.h"
#include "../functor.h"
#include "concepts.h"
#include "sendercoroutine.h"

namespace Engine {
namespace Execution {

    template <AnySender Sender, typename R, typename... V>
    struct SenderState;

    template <typename R, typename... V>
    struct Sender {

        static void destroyState(SenderStateBase<R, V...> *state)
        {
            state->destroy();
        }
        using StatePtr = std::unique_ptr<SenderStateBase<R, V...>, Functor<&destroyState>>;

        Sender() = default;
        Sender(StatePtr state)
            : mState(std::move(state))
        {
        }

        template <AnySender Inner>
        Sender(Inner &&sender)
            : mState(new SenderState<Inner, R, V...>(std::forward<Inner>(sender)))
        {
        }

        Sender &operator=(StatePtr state)
        {
            mState = std::move(state);
            return *this;
        }

        StatePtr release()
        {
            return std::move(mState);
        }

        StatePtr connect(SenderReceiver<R, V...> &receiver)
        {
            mState->connect(receiver);
            return std::move(mState);
        }

        struct state : SenderReceiver<R, V...> {

            state(StatePtr state)
                : mState(std::move(state))
            {
            }

            void start()
            {
                mState->start();
            }

            void stop()
            {
                mState->stop();
            }

            friend void tag_invoke(visit_state_t, state *state, CallableView<void(const StateDescriptor &)> visitor)
            {
                if (state)
                    state->mState->visitState(visitor);
                else
                    visitor(Execution::State::Text { "Sender" });
            }

        protected:
            void connect()
            {
                mState->connect(*this);
            }

            StatePtr mState;
        };

        template <typename Rec, typename Base = SenderReceiver<R, V...>>
        struct VirtualSenderState : VirtualState<Base, Rec> {

            using VirtualState<Base, Rec>::VirtualState;
        };

        template <typename Rec>
        struct state_helper : VirtualSenderState<Rec, state> {
            state_helper(Rec &&rec, StatePtr statePtr)
                : VirtualSenderState<Rec, state>(std::forward<Rec>(rec), std::move(statePtr))
            {
                this->connect();
            }
        };

        using is_sender = void;

        using result_type = R;
        template <template <typename...> typename Tuple>
        using value_types = Tuple<V...>;

        template <typename Rec, std::same_as<Sender> T> // Necessary to prevent implicit conversion
        friend auto tag_invoke(Execution::connect_t, T &&sender, Rec &&rec)
        {
            assert(sender.mState);
            return state_helper<Rec> { std::forward<Rec>(rec), std::move(sender.mState) };
        }

        using promise_type = CoroutineSenderState<R, V...>;

        StatePtr mState;
    };

    template <AnySender Sender, typename R, typename... V>
    struct SenderState : SenderStateBase<R, V...> {

        using State = connect_result_t<Sender, SenderReceiver<R, V...> &>;

        SenderState(Sender &&sender)
            : mData(std::forward<Sender>(sender))
        {
        }

        void connect(SenderReceiver<R, V...> &rec) override
        {
            Sender sender = std::forward<Sender>(std::get<Sender>(mData));
            mData.template emplace<State>(
                DelayedConstruct<State> { [&]() { return connect(std::forward<Sender>(sender), rec); } });
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
            visit_state(std::holds_alternative<State>(mData) ? &std::get<State>(mData) : nullptr, visitor);
        }

        std::variant<Sender, State> mData;
    };

}
}