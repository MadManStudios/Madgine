#pragma once

#include "../genericresult.h"

#include "concepts.h"

#include "virtualstate.h"

#include "flag.h"

#include "stop_callback.h"
#include "stop_source.h"

#include "stoppable.h"

namespace Engine {
namespace Execution {

    template <auto... cpos>
    struct Lifetime {

        Lifetime()
        {
        }

        ~Lifetime()
        {
            assert(!mReceiver);
            assert(mCount == 0);
        }

        template <Sender Sender>
        void attach(Sender &&sender)
        {
            if (mReceiver) {
                (new attach_state<stoppable_t::sender<Sender>> { std::forward<Sender>(sender) | stoppable, *mReceiver })->start();
            }
        }

        bool end()
        {
            if (!mReceiver)
                return false;
            return mReceiver->mStopSource.request_stop();
        }

        bool running() const
        {
            return mReceiver;
        }

        auto &finished()
        {
            return mFinished;
        }

        using is_sender = void;

        using result_type = GenericResult;
        template <template <typename...> typename Tuple>
        using value_types = Tuple<>;

        template <typename Rec>
        friend auto tag_invoke(connect_t, Lifetime &lifetime, Rec &&rec)
        {
            return state<Rec>(std::forward<Rec>(rec), lifetime);
        }

    private:
        void increaseCount()
        {
            mFinished.reset();
            ++mCount;
        }

        void decreaseCount()
        {
            if (mCount.fetch_sub(1) == 1) {
                mFinished.emplace();
            }
        }

        struct LifetimeReceiver;

        template <typename Sender>
        struct attach_state;

        template <typename Sender>
        struct attach_receiver : execution_receiver<> {

            attach_receiver(attach_state<Sender> *state)
                : mState(state)
            {
            }

            void set_value()
            {
                mState->mReceiver.decreaseCount();
                delete mState;
            }

            template <typename... V>
            [[nodiscard]] std::monostate set_value(V &&...)
            {
                mState->mReceiver.decreaseCount();
                delete mState;
                return {};
            }
            void set_done()
            {
                mState->mReceiver.decreaseCount();
                delete mState;
            }
            template <typename... R>
            [[nodiscard]] std::monostate set_error(R &&...)
            {
                mState->mReceiver.decreaseCount();
                delete mState;
                return {};
            }

            friend StopToken tag_invoke(get_stop_token_t, attach_receiver<Sender> &rec)
            {
                return rec.mState->mReceiver.mStopToken;
            }

            template <typename CPO, typename... Args>
                requires(is_tag_invocable_v<CPO, LifetimeReceiver &, Args...>)
            friend auto tag_invoke(CPO f, attach_receiver &rec, Args &&...args) noexcept(is_nothrow_tag_invocable_v<CPO, LifetimeReceiver &, Args...>)
                -> tag_invoke_result_t<CPO, LifetimeReceiver &, Args...>
            {
                return tag_invoke(f, rec.mState->mReceiver, std::forward<Args>(args)...);
            }

            attach_state<Sender> *mState;
        };

        template <typename Sender>
        struct attach_state {
            attach_state(Sender &&sender, LifetimeReceiver &receiver)
                : mReceiver(receiver)
                , mState(connect(std::forward<Sender>(sender), attach_receiver<Sender> { this }))
            {
            }
            void start()
            {
                mReceiver.increaseCount();
                mState.start();
            }

            LifetimeReceiver &mReceiver;
            connect_result_t<Sender, attach_receiver<Sender>> mState;
        };

        struct LifetimeReceiver : VirtualReceiverBaseEx<type_pack<>, type_pack<>, cpos...> {

            LifetimeReceiver(Lifetime &lifetime)
                : mStopToken(mStopSource.get_token())
                , mLifetime(lifetime)
            {
            }

            void increaseCount()
            {
                ++mCount;
            }

            void decreaseCount()
            {
                if (mCount.fetch_sub(1) == 1) {
                    mLifetime.decreaseCount();
                    this->set_value();
                }
            }

            std::atomic<uint32_t> mCount = 1;
            StopSource mStopSource;
            StopToken mStopToken;
            Lifetime &mLifetime;
        };

        template <typename Rec>
        struct state : VirtualState<Rec, LifetimeReceiver>, StopCallback {
            state(Rec &&rec, Lifetime &lifetime)
                : VirtualState<Rec, LifetimeReceiver>(std::forward<Rec>(rec), lifetime)
            {
                bool registered = this->mStopSource.registerCallback(this);
                assert(registered);
            }

            void start()
            {
                this->mLifetime.increaseCount();
                assert(!this->mLifetime.mReceiver);
                this->mLifetime.mReceiver = this;
            }

            void stop()
            {
                this->mStopSource.request_stop();            
            }

            void stopRequested() override
            {
                assert(this->mLifetime.mReceiver == this);
                this->mLifetime.mReceiver = nullptr;
                this->decreaseCount();
            }
        };

        LifetimeReceiver *mReceiver = nullptr;
        std::atomic<uint32_t> mCount = 0;
        Flag<> mFinished;
    };

}
}