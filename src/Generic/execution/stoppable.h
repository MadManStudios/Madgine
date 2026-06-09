#pragma once

#include "concepts.h"
#include "stop_callback.h"
#include "stop_source.h"
#include "storage.h"

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
                [[maybe_unused]] uint8_t state = mFlags.exchange(0);
                assert(!(state & FINISH_STARTED) == !(state & FINISH_ENDED));
                assert(!(state & STOP_STARTED) == !(state & STOP_ENDED));

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

            friend void tag_invoke(visit_state_t, state *state, auto &&visitor)
            {
                visit_state(state ? &state->mState : nullptr, visitor);
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
            friend auto tag_invoke(connect_t connect, sender &&sender, Rec &&rec)
            {
                if constexpr (!tag_invocable<get_stop_token_t, Rec &>)
                    return tag_invoke(connect, std::forward<Sender>(sender.mSender), std::forward<Rec>(rec));
                else
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

    struct stop_when_t {

        template <typename Rec, typename Inner, typename Trigger>
        struct state;

        template <typename Rec, typename Inner, typename Trigger>
        struct receiver {

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

            template <typename CPO, typename... Args>
            friend auto tag_invoke(CPO f, receiver &rec, Args &&...args)
                -> tag_invoke_result_t<CPO, Rec &, Args...>
            {
                return f(rec.mState->mRec, std::forward<Args>(args)...);
            }

            friend StopToken tag_invoke(get_stop_token_t, receiver &rec)
            {
                return rec.mState->mStopSource.get_token();
            }

            state<Rec, Inner, Trigger> *mState;
        };

        template <typename Rec, typename Inner, typename Trigger>
        struct stop_receiver {
            void set_value(auto &&...)
            {
                mState->stop();
            }

            void set_done()
            {
                mState->stop();
            }

            template <typename... R>
            void set_error(R &&...result)
            {
                mState->stop();
            }

            template <typename CPO, typename... Args>
            friend auto tag_invoke(CPO f, stop_receiver &rec, Args &&...args)
                -> tag_invoke_result_t<CPO, Rec &, Args...>
            {
                return f(rec.mState->mRec, std::forward<Args>(args)...);
            }

            friend StopToken tag_invoke(get_stop_token_t, stop_receiver &rec)
            {
                return rec.mState->mStopSource.get_token();
            }

            state<Rec, Inner, Trigger> *mState;
        };

        template <typename Rec, typename Inner, typename Trigger>
        struct state {

            using inner_state = connect_result_t<stoppable_t::sender<Inner>, receiver<Rec, Inner, Trigger>>;
            using stop_state = connect_result_t<stoppable_t::sender<Trigger>, stop_receiver<Rec, Inner, Trigger>>;

            state(Rec &&rec, Inner &&sender, Trigger &&trigger)
                : mRec(std::forward<Rec>(rec))
                , mInnerState(connect(std::forward<Inner>(sender) | stoppable, receiver<Rec, Inner, Trigger> { this }))
                , mStopState(connect(std::forward<Trigger>(trigger) | stoppable, stop_receiver<Rec, Inner, Trigger> { this }))
            {
            }

            ~state() { }

            void start()
            {
                mInnerState.start();
                mStopState.start();
            }

            template <typename... V>
            void set_value(V &&...values)
            {
                if (mStopSource.request_stop()) {
                    mResult.set_value(std::forward<V>(values)...);
                }
                signal();
            }
            void set_done()
            {
                if (mStopSource.request_stop()) {
                    mResult.set_done();
                }
                signal();
            }
            template <typename... R>
            void set_error(R &&...results)
            {
                if (mStopSource.request_stop()) {
                    mResult.set_error(std::forward<R>(results)...);
                }
                signal();
            }

            void stop()
            {
                mStopSource.request_stop();
                signal();
            }

            void signal()
            {
                if (mFinished.test_and_set()) {
                    if (mResult.is_null()) {
                        mRec.set_value();
                    } else {
                        ResultStorage<Inner> { std::move(mResult) }.reproduce(mRec);
                    }
                }
            }

            friend void tag_invoke(visit_state_t, state *state, auto &&visitor)
            {
                visit_state(state ? &state->mStopState : nullptr, visitor);
                visit_state(state ? &state->mInnerState : nullptr, visitor);
            }

            ResultStorage<Inner> mResult;
            Rec mRec;
            StopSource mStopSource;
            inner_state mInnerState;
            stop_state mStopState;
            // stop_callback<> mPropagateCallback;
            std::atomic_flag mFinished;
        };

        template <AnySender Inner, AnySender Trigger>
        struct sender : algorithm_sender<Inner> {

            template <template <typename...> typename Tuple>
            using value_types = typename Inner::template value_types<Tuple>;

            template <typename Rec>
            friend auto tag_invoke(connect_t, sender &&sender, Rec &&rec)
            {
                return state<Rec, Inner, Trigger> { std::forward<Rec>(rec), std::forward<Inner>(sender.mSender), std::forward<Trigger>(sender.mTrigger) };
            }

            Trigger mTrigger;
        };

        template <AnySender Inner, AnySender Trigger>
        friend auto tag_invoke(stop_when_t, Inner &&inner, Trigger &&trigger)
        {
            return sender<Inner, Trigger> { { {}, std::forward<Inner>(inner) }, std::forward<Trigger>(trigger) };
        }

        template <AnySender Inner, AnySender Trigger>
            requires tag_invocable<stop_when_t, Inner, Trigger>
        auto operator()(Inner &&sender, Trigger &&trigger) const
            noexcept(is_nothrow_tag_invocable_v<stop_when_t, Inner, Trigger>)
                -> tag_invoke_result_t<stop_when_t, Inner, Trigger>
        {
            return tag_invoke(*this, std::forward<Inner>(sender), std::forward<Trigger>(trigger));
        }

        template <AnySender Trigger>
        auto operator()(Trigger &&trigger) const
        {
            return pipable_from_right(*this, std::forward<Trigger>(trigger));
        }
    };

    inline constexpr stop_when_t stop_when;

}
}