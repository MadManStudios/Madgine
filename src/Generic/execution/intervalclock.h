#pragma once

#include "../genericresult.h"
#include "binding.h"
#include "concepts.h"
#include "statedescriptor.h"

namespace Engine {
namespace Execution {

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
                        if ((*it)->mStartTime + (*it)->mDuration <= mLastTick) {
                            doneWaits.push_back(*it);
                            it = mWaitStates.erase(it);
                        } else {
                            ++it;
                        }
                    }
                }
                for (WaitState *waitState : doneWaits)
                    waitState->continueExecution(now);
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
                mStartTime = mClock->mLastTick;
                std::lock_guard guard { mClock->mMutex };
                mClock->mWaitStates.push_back(this);
            }

            bool stop()
            {
                std::lock_guard guard { mClock->mMutex };
                return std::erase(mClock->mWaitStates, this) == 1;
            }

            virtual void continueExecution(Timepoint now) = 0;

            IntervalClock *mClock = nullptr;
            std::chrono::steady_clock::duration mDuration;
            Timepoint mStartTime;
        };

        template <Execution::Binding<IntervalClock &> Binding, typename Rec, bool fixed>
        struct state : WaitState, Execution::base_state<Rec> {

            using Acc = std::conditional_t<fixed, std::chrono::steady_clock::duration &, std::chrono::steady_clock::duration>;

            state(Binding &&binding, Rec &&rec, std::chrono::steady_clock::duration duration, std::chrono::steady_clock::duration durationOverride, Acc acc)
                : WaitState(duration)
                , Execution::base_state<Rec>(std::forward<Rec>(rec))
                , mBinding(std::forward<Binding>(binding))
                , mDebugDuration(durationOverride > 0s ? durationOverride : duration)
                , mAcc(acc)
            {
            }

            void start()
            {
                if constexpr (fixed) {
                    if (this->mDuration <= 0s) {
                        assert(mAcc >= this->mDuration);
                        mAcc -= mDebugDuration;
                        this->set_value();
                        return;
                    }
                }
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

            void continueExecution(Timepoint now) override
            {
                if constexpr (fixed) {
                    mAcc += (now - this->mStartTime);
                    assert(mAcc >= this->mDuration);
                    mAcc -= this->mDuration;
                    this->set_value();
                } else {
                    this->set_value(std::chrono::duration_cast<std::chrono::microseconds>(now - this->mStartTime));
                }
            }

            friend auto tag_invoke(Execution::visit_state_t, state *state, auto &&visitor)
            {
                if (!state) {
                    visitor(Execution::State::Text { "Wait..." });
                } else {
                    std::chrono::duration duration = state->mDebugDuration;
                    if (duration.count() == 0) {
                        double progress = 0.0;
                        if (state) {
                            visitor(Execution::State::Marker {});
                            if (state->mClock)
                                progress = std::chrono::duration_cast<std::chrono::duration<double>>(state->mClock->lastTick().time_since_epoch()).count();
                        }
                        visitor(Execution::State::Progress { fmodf(static_cast<float>(progress), 1.0f), true });
                    } else {
                        std::stringstream title;
                        title << "Waiting ";
                        if (duration.count() < 1000) {
                            title << duration;
                        } else if (duration.count() < 1000000) {
                            title << std::chrono::duration_cast<std::chrono::duration<float, std::micro>>(duration);
                        } else if (duration.count() < 1000000000) {
                            title << std::chrono::duration_cast<std::chrono::duration<float, std::milli>>(duration);
                        } else {
                            title << std::chrono::duration_cast<std::chrono::duration<float>>(duration);
                        }

                        visitor(Execution::State::BeginBlock { title.str() });

                        float progress = 0.0f;

                        if (state) {
                            std::chrono::steady_clock::duration elapsed = std::chrono::duration_cast<std::chrono::steady_clock::duration>(state->mClock->lastTick() - state->mStartTime) + state->mAcc;
                            progress = static_cast<float>(elapsed.count()) / duration.count();

                            visitor(Execution::State::Marker {});
                        }

                        visitor(Execution::State::Progress { progress });

                        visitor(Execution::State::EndBlock {});
                    }
                }
            }

            Binding mBinding;
            std::chrono::steady_clock::duration mDebugDuration;
            Acc mAcc;
        };

        template <Execution::Binding<IntervalClock &> Binding, bool fixed>
        struct sender : Execution::base_sender {

            using result_type = GenericResult;
            template <template <typename...> typename Tuple>
            using value_types = std::conditional_t<fixed, type_pack<>, type_pack<std::chrono::microseconds>>::template instantiate<Tuple>;

            template <typename Rec>
            friend auto tag_invoke(Execution::connect_t, sender &&sender, Rec &&rec)
            {
                return state<Binding, Rec, fixed> { std::forward<Binding>(sender.mBinding), std::forward<Rec>(rec), sender.mDuration, sender.mDurationOverride, sender.mAcc };
            }

            Binding mBinding;
            std::chrono::steady_clock::duration mDuration;
            std::chrono::steady_clock::duration mDurationOverride;
            std::conditional_t<fixed, std::chrono::steady_clock::duration &, std::chrono::steady_clock::duration> mAcc;
        };

        /* auto wait(Timepoint until)
        {
            return sender { {}, this, until };
        }*/

        auto wait(std::chrono::steady_clock::duration duration, std::chrono::steady_clock::duration durationOverride = -1s, std::chrono::steady_clock::duration acc = 0s)
        {
            return sender<Execution::ConstantBinding<IntervalClock &>, false> { {}, Execution::ConstantBinding<IntervalClock &> { *this }, duration, durationOverride, acc };
        }

        auto wait_fixed(std::chrono::steady_clock::duration duration, std::chrono::steady_clock::duration &acc)
        {
            return sender<Execution::ConstantBinding<IntervalClock &>, true> { {}, Execution::ConstantBinding<IntervalClock &> { *this }, duration - acc, duration, acc };
        }

        template <Execution::Binding<IntervalClock &> Binding>
        static auto wait(Binding &&self, std::chrono::steady_clock::duration duration)
        {
            return sender<Binding, false> { {}, std::forward<Binding>(self), duration };
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
}