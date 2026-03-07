#pragma once

#include "../callable_view.h"
#include "concepts.h"
#include "statedescriptor.h"
#include "storage.h"
#include "virtualstate.h"
#include "stoppable.h"

namespace Engine {
namespace Execution {

    template <typename R, typename... V>
    using SenderReceiver = VirtualReceiverBaseEx<type_pack<R>, type_pack<V...>, get_stop_token>;

    template <typename R, typename... V>
    struct SenderStateBase {
        virtual ~SenderStateBase() = default;

        virtual void connect(SenderReceiver<R, V...> &rec) = 0;
        virtual void start() = 0;
        virtual void stop() = 0;
        virtual void destroy()
        {
            delete this;
        }

        virtual void visitState(CallableView<void(const StateDescriptor &)> visitor) = 0;
    };

    template <typename R, typename... V>
    struct CoroutineSenderState;

    template <typename R, typename... V>
    struct CoroutineSenderStateBase : SenderStateBase<R, V...> {

        Sender<R, V...> get_return_object() {
            return typename Sender<R, V...>::StatePtr { this };
        }

        void connect(SenderReceiver<R, V...> &rec) override
        {
            mReceiver = &rec;
        }

        void start() override
        {
            std::coroutine_handle<CoroutineSenderStateBase>::from_promise(*this).resume();
        }
        void stop() override
        {
        }
        void destroy() override
        {
            std::coroutine_handle<CoroutineSenderStateBase>::from_promise(*this).destroy();
        }

        void visitState(CallableView<void(const Execution::StateDescriptor &)> visitor) override
        {
            // visitor(Execution::State::SubLocation { mDebugLocation });
            throw 0;
        }

        struct FinalSuspend {
            bool await_ready() noexcept
            {
                return false;
            }

            void await_suspend(std::coroutine_handle<CoroutineSenderState<R, V...>> handle) noexcept
            {
                handle.promise().set_value();
            }

            void await_resume() noexcept
            {
                std::terminate();
            }
        };

        std::suspend_always initial_suspend() noexcept
        {
            return {};
        }

        FinalSuspend final_suspend() noexcept
        {
            return {};
        }

        void unhandled_exception()
        {
            throw;
        }

        void set_error(R result)
        {
            mReceiver->set_error(std::forward<R>(result));
        }

        void set_done()
        {
            mReceiver->set_done();
        }

        template <typename Sender>
        struct SenderAwaitableSender;

        template <typename Sender>
        struct SenderAwaitableReceiver {

            template <typename... V2>
            void set_value(V2 &&...value)
            {
                mState->set_value(std::forward<V2>(value)...);
            }

            void set_done()
            {
                mState->set_done();
            }

            template <typename... R2>
            void set_error(R2 &&...result)
            {
                mState->set_error(std::forward<R2>(result)...);
            }

            template <typename CPO, typename... Args>
                requires(is_tag_invocable_v<CPO, SenderReceiver<R, V...> &, Args...>)
            friend auto tag_invoke(CPO f, SenderAwaitableReceiver &rec, Args &&...args) noexcept(is_nothrow_tag_invocable_v<CPO, SenderReceiver<R, V...> &, Args...>)
                -> tag_invoke_result_t<CPO, SenderReceiver<R, V...> &, Args...>
            {
                return tag_invoke(f, *rec.mCoroutine->mReceiver, std::forward<Args>(args)...);
            }

            SenderAwaitableSender<Sender> *mState;
            CoroutineSenderStateBase *mCoroutine;
        };

        template <typename Sender>
        struct SenderAwaitableSender {

            static auto buildState(SenderAwaitableSender *self, Sender &&sender, CoroutineSenderStateBase *state)
            {
                return Execution::connect(std::forward<Sender>(sender) | Execution::stoppable, SenderAwaitableReceiver<Sender> { self, state });
            }

            using S = std::invoke_result_t<decltype(&SenderAwaitableSender::buildState), SenderAwaitableSender *, Sender, std::nullptr_t>;

            SenderAwaitableSender(Sender &&sender, CoroutineSenderStateBase *state)
                : mState(buildState(this, std::forward<Sender>(sender), state))
            {
            }

            bool await_ready()
            {
                mState.start();
                return mFlag.test() && mResult.is_value();
            }

            bool await_suspend(std::coroutine_handle<CoroutineSenderState<R, V...>> coroutine)
            {
                mCoroutine = coroutine;
                if (mFlag.test_and_set()) {
                    if (mResult.is_value()) {
                        return false;
                    } else if (mResult.is_error()) {
                        mResult.reproduce_error(mCoroutine.promise());
                    } else {
                        mCoroutine.promise().set_done();
                    }
                }
                return true;
            }

            ValueStorage<Sender> await_resume()
            {
                return std::move(mResult).value();
            }

            template <typename... V2>
            void set_value(V2 &&...v)
            {
                mResult.set_value(std::forward<V2>(v)...);
                if (mFlag.test_and_set())
                    mCoroutine.resume();
            }

            void set_done()
            {
                mResult.set_done();
                if (mFlag.test_and_set())
                    mCoroutine.promise().set_done();
            }

            template <typename... R2>
            void set_error(R2 &&...error)
            {
                mResult.set_error(std::forward<R2>(error)...);
                if (mFlag.test_and_set())
                    mResult.reproduce_error(mCoroutine.promise());
            }

        private:
            S mState;
            std::atomic_flag mFlag = ATOMIC_FLAG_INIT;
            std::coroutine_handle<CoroutineSenderState<R, V...>> mCoroutine;
            Execution::ResultStorage<Sender> mResult;
        };

        template <typename T>
        decltype(auto) await_transform(T &&awaitable)
        {
            if constexpr (AnySender<std::remove_reference_t<T>>) {
                return SenderAwaitableSender<T> { std::forward<T>(awaitable), this };
            } else {
                return std::forward<T>(awaitable);
            }
        }

        SenderReceiver<R, V...> *mReceiver = nullptr;
    };

    template <typename R, typename... V>
    struct CoroutineSenderState : CoroutineSenderStateBase<R, V...> {

        void return_value(V &&...value)
        {
            construct(mResult, std::forward<V>(value)...);
        }

        void set_value()
        {
            std::tuple<V...> result = std::move(*mResult);
            destruct(mResult);
            TupleUnpacker::invokeExpand(LIFT(this->mReceiver->set_value, this), std::move(result));
        }

        ManualLifetime<std::tuple<V...>> mResult;
    };

    template <typename R>
    struct CoroutineSenderState<R> : CoroutineSenderStateBase<R> {

        void return_void()
        {
        }

        void set_value()
        {
            this->mReceiver->set_value();
        }
    };

}
}
