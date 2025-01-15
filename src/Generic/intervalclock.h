#pragma once

#include "execution/concepts.h"
#include "execution/statedescriptor.h"

#include "execution/stop_callback.h"

#include "execution/algorithm.h"

namespace Engine {

namespace Execution {
    struct get_debug_data_t;
}

template <typename Timepoint = std::chrono::steady_clock::time_point>
struct IntervalClock {

    IntervalClock(Timepoint start)
        : mLastTick(start)
    {
    }

    std::chrono::microseconds tick(Timepoint now)
    {
        std::chrono::microseconds elapsed = std::chrono::duration_cast<std::chrono::microseconds>(now - mLastTick);
        mLastTick = now;

        if (elapsed.count() > 0) {
            std::vector<WaitState *> doneWaits;
            {
                std::lock_guard guard { mMutex };
                for (auto it = mWaitStates.begin(); it != mWaitStates.end();) {
                    if ((*it)->mWaitUntil <= mLastTick) {
                        doneWaits.push_back(*it);
                        it = mWaitStates.erase(it);
                    } else {
                        ++it;
                    }
                }
            }
            for (WaitState *waitState : doneWaits)
                waitState->continueExecution(elapsed);
        }

        return elapsed;
    }

    struct WaitState {

        WaitState(std::chrono::steady_clock::duration duration)
            : mDuration(duration)
        {
        }

        virtual void start(IntervalClock *clock)
        {
            mClock = clock;
            mWaitUntil = mClock->mLastTick + mDuration;
            std::lock_guard guard { mClock->mMutex };
            mClock->mWaitStates.push_back(this);
        }

        bool cancel()
        {
            std::lock_guard guard { mClock->mMutex };
            return std::erase(mClock->mWaitStates, this) == 1;
        }

        virtual void continueExecution(std::chrono::microseconds elapsed) = 0;

        friend const void *tag_invoke(const Execution::get_debug_data_t &, WaitState &state)
        {
            return &state;
        }

        IntervalClock *mClock = nullptr;
        std::chrono::steady_clock::duration mDuration;
        Timepoint mWaitUntil;
    };

    template <typename Rec>
    struct receiver : Execution::algorithm_receiver<Rec> {

        receiver(Rec &&rec, WaitState *state)
            : Execution::algorithm_receiver<Rec> { std::forward<Rec>(rec) }
            , mState(state)
        {
        }

        void set_value(IntervalClock *clock)
        {
            mState->start(clock);
        }

        void set_value(IntervalClock &clock)
        {
            set_value(&clock);
        }

        WaitState *mState;
    };

    template <typename Inner, typename Rec>
    struct state : WaitState, Execution::base_state<Rec> {

        state(Inner &&inner, Rec &&rec, std::chrono::steady_clock::duration duration)
            : WaitState(duration)
            , Execution::base_state<Rec>(std::forward<Rec>(rec))
            , mInnerState(Execution::connect(std::forward<Inner>(inner), receiver<Rec &> { this->mRec, this }))
            , mCallback(finally_cb { this->mRec })
        {
        }

        void start()
        {
            mInnerState.start();
        }

        virtual void start(IntervalClock *clock) override
        {
            std::stop_token st = Execution::get_stop_token(this->mRec);
            if (st.stop_requested()) {
                this->mRec.set_done();
            } else {
                WaitState::start(clock);
                mCallback.start(std::move(st), stop_cb { this });
            }
        }

        virtual void continueExecution(std::chrono::microseconds elapsed) override
        {
            mCallback.finish(std::move(elapsed));
        }

        friend auto tag_invoke(Execution::visit_state_t, state &state, const std::chrono::steady_clock::duration &duration, auto &&visitor)
        {
            if (duration.count() == 0) {
                visitor(Execution::State::Text { "Yield" });
            } else {
                std::string title;
                /* if (dur.count() < 1000) {
                    title = std::format("Waiting {} ns", dur.count());
                } else if (dur.count() < 1000000) {
                    title = std::format("Waiting {:.3f} us", std::chrono::duration_cast<std::chrono::duration<float, std::micro>>(dur).count());
                } else if (dur.count() < 1000000000) {
                    title = std::format("Waiting {:.4f} ms", std::chrono::duration_cast<std::chrono::duration<float, std::milli>>(dur).count());
                } else {
                    title = std::format("Waiting {:.4f} s", std::chrono::duration_cast<std::chrono::duration<float>>(dur).count());
                }*/
                title = "Waiting";

                visitor(Execution::State::BeginBlock { title });

                float progress = 0.0f;

                std::chrono::steady_clock::duration remaining = std::chrono::duration_cast<std::chrono::steady_clock::duration>(state.mWaitUntil - state.mClock->lastTick());
                progress = 1.0f - (static_cast<float>(remaining.count()) / duration.count());
                
                visitor(Execution::State::Progress { progress });

                visitor(Execution::State::EndBlock {});
            }
        }

        struct stop_cb {
            bool operator()()
            {
                return mState->cancel();
            }

            WaitState *mState;
        };

        struct finally_cb {
            void operator()(std::chrono::microseconds elapsed)
            {
                if (Execution::get_stop_token(mRec).stop_requested())
                    mRec.set_done();
                else
                    mRec.set_value(std::move(elapsed));
            }

            void operator()(Execution::cancelled_t)
            {
                assert(Execution::get_stop_token(mRec).stop_requested());
                mRec.set_done();
            }

            Rec &mRec;
        };

        Execution::connect_result_t<Inner, receiver<Rec &>> mInnerState;
        Execution::stop_callback<stop_cb, finally_cb> mCallback;
    };

    template <typename Inner>
    struct sender : Execution::algorithm_sender<Inner> {

        using result_type = typename Inner::result_type;
        template <template <typename...> typename Tuple>
        using value_types = Tuple<std::chrono::microseconds>;

        template <typename Rec>
        friend auto tag_invoke(Execution::connect_t, sender &&sender, Rec &&rec)
        {
            return state<Inner, Rec> { std::forward<Inner>(sender.mSender), std::forward<Rec>(rec), sender.mDuration };
        }

        friend auto tag_invoke(Execution::visit_sender_t, sender &sender)
        {
            return sender.mDuration;
        }

        static constexpr size_t debug_start_increment = 1;
        static constexpr size_t debug_operation_increment = 1;
        static constexpr size_t debug_stop_increment = 1;

        std::chrono::steady_clock::duration mDuration;
    };

    /* auto wait(Timepoint until)
    {
        return sender { {}, this, until };
    }*/

    auto wait(std::chrono::steady_clock::duration duration)
    {
        return sender<Execution::just_t::sender<IntervalClock *>> { { {}, Execution::just(this) }, duration };
    }

    template <typename Sender>
    static auto wait(Sender &&inner, std::chrono::steady_clock::duration duration)
    {
        return sender<Sender> { { {}, std::forward<Sender>(inner) }, duration };
    }

    const Timepoint &lastTick() const
    {
        return mLastTick;
    }

private:
    Timepoint mLastTick;

    std::mutex mMutex;
    std::vector<WaitState *> mWaitStates;
};

}