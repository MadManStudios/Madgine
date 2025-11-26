#pragma once

#include "concepts.h"

#include "virtualstate.h"

#include "flag.h"

#include "stop_callback.h"
#include "stop_source.h"

#include "stoppable.h"

#include "binding.h"

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

        template <typename T>
        struct sender {
            using is_sender = void;

            using result_type = void;
            template <template <typename...> typename Tuple>
            using value_types = Tuple<>;

            template <typename Rec>
            friend auto tag_invoke(connect_t, sender<T> &&sender, Rec &&rec)
            {
                return state<Rec>(std::forward<Rec>(rec), sender.mLifetime, sender.mPtr, std::forward<T>(sender.mT));
            }

            Lifetime &mLifetime;
            T mT;
            BindingPtr<T> &mPtr;
        };

        template <typename T>
        sender<T> bound(BindingPtr<T> &ptr, T &&t)
        {
            return sender<T> { *this, std::forward<T>(t), ptr };
        }

        using is_sender = void;

        using result_type = void;
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
                return f(rec.mState->mReceiver, std::forward<Args>(args)...);
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
        struct state : VirtualState<LifetimeReceiver, Rec>, StopCallback {
            state(Rec &&rec, Lifetime &lifetime)
                : VirtualState<LifetimeReceiver, Rec>(std::forward<Rec>(rec), lifetime)
            {
                bool registered = this->mStopSource.registerCallback(this);
                assert(registered);
            }

            template <typename T>
            state(Rec &&rec, Lifetime &lifetime, BindingPtr<T> &ptr, T &&t)
                : state(std::forward<Rec>(rec), lifetime)
            {
                ptr = BindingPtr<T> { new BindingPoint<T>(*this, std::forward<T>(t)) };
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

            friend auto tag_invoke(Execution::visit_state_t, state *state, auto &&visitor)
            {
                visitor(State::BeginBlock { "Lifetime" });
                if (state) {
                    visitor(State::Marker {});
                    visitor(State::Text { "Active: " + std::to_string(state->mCount) });
                }
                visitor(State::EndBlock {});
            }
        };

        template <typename T>
        struct BindingPoint : StopCallback, BindingBridgeBase<T> {

            BindingPoint(LifetimeReceiver &receiver, T &&t)
                : mReceiver(receiver)
                , mT(std::forward<T>(t))
            {
                ++this->mRefCount;
                mReceiver.increaseCount();
                if (!mReceiver.mStopSource.registerCallback(this)) {
                    stopRequested();
                }
            }

            void decreaseCount()
            {
                if (mStrongRefCount.fetch_sub(1) == 1) {
                    mReceiver.decreaseCount();
                }
            }

            void stopRequested() override
            {
                decreaseCount();

                if (this->mRefCount.fetch_sub(1) == 1) {
                    delete this;
                }
            }

            bool access(CallableView<bool(const T &)> callback) override
            {
                uint32_t refCount = mStrongRefCount;
                if (refCount > 0) {
                    if (mReceiver.mStopSource.stop_requested())
                        return false;

                    while (refCount > 0) {
                        if (mStrongRefCount.compare_exchange_weak(refCount, refCount + 1)) {
                            bool result = callback(mT);
                            decreaseCount();
                            return result;
                        }
                    }
                }
                return false;
            }

        private:
            LifetimeReceiver &mReceiver;
            T mT;

            std::atomic<uint32_t> mStrongRefCount = 1;
        };

        LifetimeReceiver *mReceiver = nullptr;
        std::atomic<uint32_t> mCount = 0;
        Flag<> mFinished;
    };

}
}