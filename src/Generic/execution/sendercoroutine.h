#pragma once

#include "../callable_view.h"
#include "awaitablesender.h"
#include "concepts.h"
#include "statedescriptor.h"
#include "stoppable.h"
#include "storage.h"
#include "virtualstate.h"

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

        Sender<R, V...> get_return_object()
        {
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

        std::bool_constant<false> set_value(auto &&...)
        {
            return {};
        }

        std::bool_constant<true> set_error(R result)
        {
            mReceiver->set_error(std::forward<R>(result));
            return {};
        }

        std::bool_constant<true> set_done()
        {
            mReceiver->set_done();
            return {};
        }

        static decltype(auto) unpack_storage(auto &&result)
        {
            return std::forward<decltype(result)>(result).value().get();
        }

        template <typename CPO, typename... Args>
            requires(is_tag_invocable_v<CPO, SenderReceiver<R, V...> &, Args...>)
        friend auto tag_invoke(CPO f, CoroutineSenderStateBase &state, Args &&...args) noexcept(is_nothrow_tag_invocable_v<CPO, SenderReceiver<R, V...> &, Args...>)
            -> tag_invoke_result_t<CPO, SenderReceiver<R, V...> &, Args...>
        {
            return tag_invoke(f, *state.mReceiver, std::forward<Args>(args)...);
        }

        template <typename T>
        decltype(auto) await_transform(T &&awaitable)
        {
            if constexpr (AnySender<std::remove_reference_t<T>>) {
                return AwaitableSender<T, CoroutineSenderStateBase> { std::forward<T>(awaitable), *this };
            } else {
                return std::forward<T>(awaitable);
            }
        }

        SenderReceiver<R, V...> *mReceiver = nullptr;
    };

    template <typename R, typename... V>
    struct CoroutineSenderState : CoroutineSenderStateBase<R, V...> {

        void return_value(V... value)
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
