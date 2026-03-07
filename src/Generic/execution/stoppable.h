#pragma once

#include "concepts.h"
#include "stop_callback.h"

namespace Engine {
namespace Execution {

    enum ExecutionState {
        FINISH_STARTED = 1 << 0,
        FINISH_ENDED = 1 << 1,
        STOP_STARTED = 1 << 2,
        STOP_ENDED = 1 << 3
    };

    struct stoppable_t {

        template <AnySender Sender, typename Rec>
        struct state;

        template <AnySender Sender, typename Rec>
        struct receiver {
            using inner = Rec;

            template <typename CPO, typename... Args>
            friend auto tag_invoke(CPO f, receiver &rec, Args &&...args)
                -> tag_invoke_result_t<CPO, Rec &, Args...>
            {
                return f(rec.mState->mRec, std::forward<Args>(args)...);
            }

            template <typename... V>
            void set_value(V &&...value)
            {
                mState->set_value(std::forward<V>(value)...);
            }

            void set_done()
            {
                mState->set_done();
            }

            template <typename... R>
            void set_error(R &&...result)
            {
                mState->set_error(std::forward<R>(result)...);
            }

            state<Sender, Rec> *mState;
        };

        template <AnySender Sender, typename Rec>
        struct state : StopCallback {

            using InnerState = connect_result_t<Sender, receiver<Sender, Rec>>;

            state(Sender &&sender, Rec &&rec)
                : mRec(std::forward<Rec>(rec))
                , mState(connect(std::forward<Sender>(sender), receiver<Sender, Rec> { this }))
            {
            }

            void start()
            {
                if (get_stop_token(mRec)->registerCallback(this))
                    this->mState.start();
                else
                    mRec.set_done();
            }

            void stopRequested() override
            {
                // TODO also sync with start()

                uint8_t state = mFlags.fetch_or(STOP_STARTED);
                assert(!(state & STOP_STARTED));
                assert(!(state & STOP_ENDED));

                this->mState.stop();
                state = mFlags.fetch_or(STOP_ENDED);
                if ((state & FINISH_ENDED))
                    mRec.set_done();
            }

            template <typename... V>
            void set_value(V &&...value)
            {
                if (finish())
                    mRec.set_value(std::forward<V>(value)...);
            }

            void set_done()
            {
                if (finish())
                    mRec.set_done();
            }

            template <typename... R>
            void set_error(R &&...result)
            {
                if (finish())
                    mRec.set_error(std::forward<R>(result)...);
            }

        protected:
            bool finish()
            {
                uint8_t state = mFlags.fetch_or(FINISH_STARTED);
                assert(!(state & FINISH_STARTED));
                assert(!(state & FINISH_ENDED));

                bool disconnected = false;
                if (!(state & STOP_STARTED)) {
                    disconnected = get_stop_token(mRec)->unregisterCallback(this);
                }

                state = mFlags.fetch_or(FINISH_ENDED);
                assert(state & FINISH_STARTED);
                assert(!(state & FINISH_ENDED));
                if (state & STOP_ENDED)
                    mRec.set_done();
                else if (!(state & STOP_STARTED))
                    return disconnected;

                return false;
            }

        public:
            Rec mRec;
            std::atomic<uint8_t> mFlags;
            InnerState mState;
        };

        template <AnySender Sender>
        struct sender : algorithm_sender<Sender> {

            template <typename Rec>
            friend auto tag_invoke(connect_t, sender &&sender, Rec &&rec)
            {
                return state<Sender, Rec> { std::forward<Sender>(sender.mSender), std::forward<Rec>(rec) };
            }
        };

        template <AnySender Sender>
        friend auto tag_invoke(stoppable_t, Sender &&inner)
        {
            return sender<Sender> { { {}, std::forward<Sender>(inner) } };
        }

        template <typename Sender>
            requires tag_invocable<stoppable_t, Sender>
        auto operator()(Sender &&sender) const
            noexcept(is_nothrow_tag_invocable_v<stoppable_t, Sender>)
                -> tag_invoke_result_t<stoppable_t, Sender>
        {
            return tag_invoke(*this, std::forward<Sender>(sender));
        }

        template <typename Sender>
        friend auto operator|(Sender &&sender, stoppable_t s)
        {
            return tag_invoke(s, std::forward<Sender>(sender));
        }
    };

    inline constexpr stoppable_t stoppable;

}
}