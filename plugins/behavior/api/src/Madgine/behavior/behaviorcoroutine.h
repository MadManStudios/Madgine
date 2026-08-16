#pragma once

#include "Generic/closure.h"
#include "Generic/execution/awaitablesender.h"

#include "Meta/reflect/result.h"

#include "Madgine/debug/debuggablesender.h"
#include "Madgine/debug/debuglocation.h"

#include "behaviorstatebase.h"
#include "concepts.h"

namespace Engine {
namespace Behavior {

    struct MADGINE_BEHAVIOR_EXPORT CoroutineLocation {
        Debug::SenderLocation *mChild = nullptr;
        mutable Debug::Continuation mContinuation;
        IndexType<size_t> mLine;

        std::source_location mLocation;
    };

    struct MADGINE_BEHAVIOR_EXPORT BoundValueBase {
        BoundValueBase(CoroutineBehaviorState *state);
        ~BoundValueBase();

        virtual bool resumeImpl() = 0;

        BoundValueBase *mNext = nullptr;
    };

    template <typename Awaiter>
    struct CoroutineAwaiterGuard;

    struct MADGINE_BEHAVIOR_EXPORT BehaviorCoroutineHandle {
        BehaviorCoroutineHandle() = default;
        BehaviorCoroutineHandle(std::coroutine_handle<CoroutineBehaviorState> handle);

        CoroutineBehaviorState &promise() const;

        void resume() const;

    private:
        std::coroutine_handle<CoroutineBehaviorState> mHandle;
    };

    struct MADGINE_BEHAVIOR_EXPORT CoroutineBehaviorState : BehaviorStateBase, BoundValueBase {

        static Reflect::Result resolveNames(BehaviorReceiver &rec)
        {
            return {};
        }

        template <typename Arg, typename... Args>
        static Reflect::Result resolveNames(BehaviorReceiver &rec, Arg &&arg, Args &&...args)
        {
            if constexpr (requires { arg.resolve(rec); }) {
                Reflect::Result result = arg.resolve(rec);
                if (result)
                    return result;
            }
            return resolveNames(rec, std::forward<Args>(args)...);
        }

        template <typename... Args>
        CoroutineBehaviorState(Args &&...args)
            : BoundValueBase(this)
        {
            mResolveNames = [&](BehaviorReceiver &rec) {
                return resolveNames(rec, args...);
            };
        }

        Behavior get_return_object();

        void connect(BehaviorReceiver &rec) override;
        void start() override;
        void stop() override;
        void destroy() override;

        void visitState(CB visitor) override;

        bool wantsPause();
        void pass(Closure<void()> callback);

        struct MADGINE_BEHAVIOR_EXPORT InitialSuspend {
            bool await_ready() noexcept;
            void await_suspend(std::coroutine_handle<CoroutineBehaviorState> handle, std::source_location location = std::source_location::current()) noexcept;
            void await_resume() noexcept;
        };

        struct MADGINE_BEHAVIOR_EXPORT FinalSuspend {
            bool await_ready() noexcept;
            void await_suspend(std::coroutine_handle<CoroutineBehaviorState> handle) noexcept;
            void await_resume() noexcept;
        };

        InitialSuspend initial_suspend() noexcept;
        FinalSuspend final_suspend() noexcept;

        bool resumeImpl() override;
        void return_void();
        void unhandled_exception();
        std::bool_constant<false> set_value(auto &&...)
        {
            return {};
        }
        std::bool_constant<true> set_error(Reflect::Error result);
        std::bool_constant<true> set_done();

        static decltype(auto) unpack_storage(auto &&result)
        {
            return std::forward<decltype(result)>(result).value().get();
        }

        template <typename T>
        decltype(auto) await_transform(T &&awaitable)
        {
            if constexpr (Execution::AnySender<std::remove_reference_t<T>>) {
                return CoroutineAwaiterGuard<Execution::AwaitableSender<Execution::with_debug_location_t::sender<T>, CoroutineBehaviorState, BehaviorCoroutineHandle>> { *this, std::forward<T>(awaitable) | Execution::with_debug_location(mDebugLocation.mChild), *this };
            } else if constexpr (Execution::AnyBinding<std::remove_reference_t<T>>) {
                return CoroutineAwaiterGuard<BehaviorAwaitableBinding<T>> { *this, std::forward<T>(awaitable) };
            } else {
                return CoroutineAwaiterGuard<T> { *this, std::forward<T>(awaitable) };
            }
        }

        template <typename CPO, typename... Args>
            requires(is_tag_invocable_v<CPO, BehaviorReceiver &, Args...>)
        friend auto tag_invoke(CPO f, CoroutineBehaviorState &state, Args &&...args) noexcept(is_nothrow_tag_invocable_v<CPO, BehaviorReceiver &, Args...>)
            -> tag_invoke_result_t<CPO, BehaviorReceiver &, Args...>
        {
            return tag_invoke(f, *state.mReceiver, std::forward<Args>(args)...);
        }

        friend MADGINE_BEHAVIOR_EXPORT CoroutineLocation *tag_invoke(Execution::get_debug_location_t, CoroutineBehaviorState &state);

        CoroutineLocation mDebugLocation;

        BehaviorReceiver *mReceiver = nullptr;

        Closure<Reflect::Result(BehaviorReceiver &)> mResolveNames;
    };

    template <typename Awaiter>
    struct CoroutineAwaiterGuard {
        template <typename... Args>
        CoroutineAwaiterGuard(CoroutineBehaviorState &state, Args &&...args)
            : mAwaiter(std::forward<Args>(args)...)
            , mState(state)
        {
        }

        bool await_ready(std::source_location location = std::source_location::current()) noexcept
        {
            mState.mDebugLocation.mLine = location.line();

            if (mState.wantsPause())
                return false;

            return mAwaiter.await_ready();
        }

        void await_suspend(BehaviorCoroutineHandle handle)
        {
            mState.pass([this, handle, wasPaused {mState.wantsPause()}]() {
                if (wasPaused && mAwaiter.await_ready()) {
                    handle.resume();
                } else {
                    using R = decltype(mAwaiter.await_suspend(handle));
                    if constexpr (std::same_as<bool, R>) {
                        if (!mAwaiter.await_suspend(handle)) {
                            handle.resume();
                        }
                    } else {
                        static_assert(std::same_as<void, R>);
                        mAwaiter.await_suspend(handle);
                    }
                }
            });
        }

        decltype(auto) await_resume()
        {
            return mAwaiter.await_resume();
        }

        Awaiter mAwaiter;
        CoroutineBehaviorState &mState;
    };
}
}
