#pragma once

#include "execution/binding.h"
#include "execution/concepts.h"
#include "execution/statedescriptor.h"
#include "execution/stop_callback.h"
#include "genericresult.h"

namespace Engine {

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

        void start(IntervalClock &clock)
        {
            mClock = &clock;
            mWaitUntil = mClock->mLastTick + mDuration;
            std::lock_guard guard { mClock->mMutex };
            mClock->mWaitStates.push_back(this);
        }

        bool stop()
        {
            std::lock_guard guard { mClock->mMutex };
            return std::erase(mClock->mWaitStates, this) == 1;
        }

        virtual void continueExecution(std::chrono::microseconds elapsed) = 0;

        IntervalClock *mClock = nullptr;
        std::chrono::steady_clock::duration mDuration;
        Timepoint mWaitUntil;
    };

    template <Execution::Binding<IntervalClock &> Binding, typename Rec>
    struct state : WaitState, Execution::base_state<Rec> {

        state(Binding &&binding, Rec &&rec, std::chrono::steady_clock::duration duration, std::chrono::steady_clock::duration durationOverride, std::chrono::steady_clock::duration acc)
            : WaitState(duration)
            , Execution::base_state<Rec>(std::forward<Rec>(rec))
            , mBinding(std::forward<Binding>(binding))
            , mDuration(durationOverride > 0s ? durationOverride : duration)
            , mAcc(mDuration - duration - acc)
        {
        }

        void start()
        {
            if (!Execution::access_binding(mBinding, [this](IntervalClock &clock) {
                    WaitState::start(clock);
                })) {
                throw 0;
                // this->set_error();
            }
        }

        void stop()
        {
            if (WaitState::stop())
                this->set_done();
        }

        void continueExecution(std::chrono::microseconds elapsed) override
        {
            this->set_value(std::move(elapsed));
        }

        friend auto tag_invoke(Execution::visit_state_t, state *state, auto &&visitor)
        {
            if (!state) {
                visitor(Execution::State::Text { "Wait..." });
            } else {
                std::chrono::duration duration = state->mDuration;
                if (duration.count() == 0) {
                    if (state) {
                        visitor(Execution::State::Marker {});
                    }
                    visitor(Execution::State::Text { "Yield" });
                } else {
                    std::string title;
                    if (duration.count() < 1000) {
                        title = std::format("Waiting {} ns", duration.count());
                    } else if (duration.count() < 1000000) {
                        title = std::format("Waiting {:.3f} us", std::chrono::duration_cast<std::chrono::duration<float, std::micro>>(duration).count());
                    } else if (duration.count() < 1000000000) {
                        title = std::format("Waiting {:.4f} ms", std::chrono::duration_cast<std::chrono::duration<float, std::milli>>(duration).count());
                    } else {
                        title = std::format("Waiting {:.4f} s", std::chrono::duration_cast<std::chrono::duration<float>>(duration).count());
                    }

                    visitor(Execution::State::BeginBlock { title });

                    float progress = 0.0f;

                    if (state) {
                        std::chrono::steady_clock::duration remaining = std::chrono::duration_cast<std::chrono::steady_clock::duration>(state->mWaitUntil - state->mClock->lastTick()) + state->mAcc;
                        progress = 1.0f - (static_cast<float>(remaining.count()) / duration.count());

                        visitor(Execution::State::Marker {});
                    }

                    visitor(Execution::State::Progress { progress });

                    visitor(Execution::State::EndBlock {});
                }
            }
        }

        Binding mBinding;
        std::chrono::steady_clock::duration mDuration;
        std::chrono::steady_clock::duration mAcc;
    };

    template <Execution::Binding<IntervalClock &> Binding>
    struct sender : Execution::base_sender {

        using result_type = GenericResult;
        template <template <typename...> typename Tuple>
        using value_types = Tuple<std::chrono::microseconds>;

        template <typename Rec>
        friend auto tag_invoke(Execution::connect_t, sender &&sender, Rec &&rec)
        {
            return state<Binding, Rec> { std::forward<Binding>(sender.mBinding), std::forward<Rec>(rec), sender.mDuration, sender.mDurationOverride, sender.mAcc };
        }

        Binding mBinding;
        std::chrono::steady_clock::duration mDuration;
        std::chrono::steady_clock::duration mDurationOverride;
        std::chrono::steady_clock::duration mAcc;
    };

    /* auto wait(Timepoint until)
    {
        return sender { {}, this, until };
    }*/

    auto wait(std::chrono::steady_clock::duration duration, std::chrono::steady_clock::duration durationOverride = -1s, std::chrono::steady_clock::duration acc = 0s)
    {
        return sender<Execution::ConstantBinding<IntervalClock &>> { {}, Execution::ConstantBinding<IntervalClock &> { *this }, duration, durationOverride, acc };
    }

    template <Execution::Binding<IntervalClock &> Binding>
    static auto wait(Binding &&self, std::chrono::steady_clock::duration duration)
    {
        return sender<Binding> { {}, std::forward<Binding>(self), duration };
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