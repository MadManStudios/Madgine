#pragma once

#include "Madgine/debug/debuglocation.h"

#include "Interfaces/debug/stacktrace.h"

#include "behaviorstatebase.h"

#include "Generic/closure.h"

#include "concepts.h"

namespace Engine {
namespace Behavior {

    struct MADGINE_BEHAVIOR_EXPORT CoroutineLocation : Debug::SimpleLocation {

        std::string toString() const override;
        std::map<std::string_view, ValueType> localVariables() const override;
        virtual bool wantsPause(Debug::ContinuationType type, IndexType<size_t> line) const override;

#ifndef NDEBUG
        Debug::StackTrace<1> mStacktrace;
#endif
    };

    struct MADGINE_BEHAVIOR_EXPORT BoundValueBase {
        BoundValueBase(CoroutineBehaviorState *state);
        ~BoundValueBase();

        virtual bool resumeImpl() = 0;
        virtual void suspendImpl() = 0;

        BoundValueBase *mNext = nullptr;
    };

    template <typename Awaiter>
    struct CoroutineAwaiterGuard;

    struct MADGINE_BEHAVIOR_EXPORT CoroutineBehaviorState : BehaviorStateBase, BoundValueBase {

        template <typename... Args>
        CoroutineBehaviorState(Args &&...args)
            : BoundValueBase(this)
        {
            mResolveNames = [&](BehaviorReceiver &rec) {
                return ([&]() {
                    if constexpr (requires { args.resolve(rec); }) {
                        return args.resolve(rec);
                    } else {
                        return true;
                    }
                }() && ...);
            };
        }

        Behavior get_return_object();

        void connect(BehaviorReceiver &rec) override;
        void start() override;
        void stop() override;
        void destroy() override;

        void visitState(CallableView<void(const Execution::StateDescriptor &)> visitor) override;

        struct MADGINE_BEHAVIOR_EXPORT InitialSuspend {
            bool await_ready() noexcept;
            void await_suspend(std::coroutine_handle<CoroutineBehaviorState> handle) noexcept;
            void await_resume() noexcept;
        };

        struct MADGINE_BEHAVIOR_EXPORT FinalSuspend {
            bool await_ready() noexcept;
            void await_suspend(std::coroutine_handle<CoroutineBehaviorState> handle) noexcept;
            void await_resume() noexcept;
        };

        InitialSuspend initial_suspend() noexcept;
        FinalSuspend final_suspend() noexcept;

        void resume();
        void suspend();
        bool resumeImpl() override;
        void suspendImpl() override;
        void return_void();
        void unhandled_exception();
        void set_error(BehaviorError result);
        void set_done();

        template <typename T>
        decltype(auto) await_transform(T &&awaitable)
        {
            if constexpr (Execution::Sender<std::remove_reference_t<T>>) {
                return CoroutineAwaiterGuard<BehaviorAwaitableSender<T>> { std::forward<T>(awaitable), this };
            } else if constexpr (Execution::AnyBinding<std::remove_reference_t<T>>) {
                return CoroutineAwaiterGuard<BehaviorAwaitableBinding<T>> { std::forward<T>(awaitable) };
            } else {
                return CoroutineAwaiterGuard<T> { std::forward<T>(awaitable) };
            }
        }

        CoroutineLocation mDebugLocation;

        BehaviorReceiver *mReceiver = nullptr;

        Closure<bool(BehaviorReceiver &)> mResolveNames;
    };

    template <typename Awaiter>
    struct CoroutineAwaiterGuard {
        template <typename... Args>
        CoroutineAwaiterGuard(Args &&...args)
            : mAwaiter(std::forward<Args>(args)...)
        {
        }

        bool await_ready() noexcept
        {
            return mAwaiter.await_ready();
        }

        auto await_suspend(std::coroutine_handle<CoroutineBehaviorState> handle)
        {
            using result_type = std::invoke_result_t<decltype(&Awaiter::await_suspend), Awaiter &, std::coroutine_handle<CoroutineBehaviorState>>;
            if constexpr (std::same_as<result_type, bool>) {
                bool result = mAwaiter.await_suspend(std::move(handle));
                if (result) {
                    mState = &handle.promise();
                    mState->suspend();
                }
                return result;
            } else {
                mState = &handle.promise();
                mState->suspend();
                return mAwaiter.await_suspend(std::move(handle));
            }
        }

        decltype(auto) await_resume()
        {
            if (mState)
                mState->resume();
            return mAwaiter.await_resume();
        }

        Awaiter mAwaiter;
        CoroutineBehaviorState *mState = nullptr;
    };

}
}
