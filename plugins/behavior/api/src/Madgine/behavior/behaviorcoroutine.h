#pragma once

#include "Generic/closure.h"
#include "Generic/execution/awaitablesender.h"

#include "Interfaces/debug/stacktrace.h"

#include "Meta/keyvalue/keyvalueresult.h"

#include "Madgine/debug/debuggablesender.h"
#include "Madgine/debug/debuglocation.h"

#include "behaviorstatebase.h"
#include "concepts.h"

namespace Engine {
namespace Behavior {

    struct MADGINE_BEHAVIOR_EXPORT CoroutineLocation {
        Debug::SenderLocation *mChild = nullptr;

#ifndef NDEBUG
        Debug::StackTrace<1> mStacktrace;
#endif
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

        void resume();

    private:
        std::coroutine_handle<CoroutineBehaviorState> mHandle;
    };


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
             
        bool resumeImpl() override;
        void return_void();
        void unhandled_exception();
        std::bool_constant<false> set_value(auto &&...)
        {
            return {};
        }
        std::bool_constant<true> set_error(KeyValueError result);
        std::bool_constant<true> set_done();

        static decltype(auto) unpack_storage(auto &&result)
        {
            return std::forward<decltype(result)>(result).value().get();
        }

        template <typename T>
        decltype(auto) await_transform(T &&awaitable)
        {
            if constexpr (Execution::AnySender<std::remove_reference_t<T>>) {
                return CoroutineAwaiterGuard<Execution::AwaitableSender<Execution::with_debug_location_t::sender<T>, CoroutineBehaviorState, BehaviorCoroutineHandle>> { std::forward<T>(awaitable) | Execution::with_debug_location(mDebugLocation.mChild), *this };
            } else if constexpr (Execution::AnyBinding<std::remove_reference_t<T>>) {
                return CoroutineAwaiterGuard<BehaviorAwaitableBinding<T>> { std::forward<T>(awaitable) };
            } else {
                return CoroutineAwaiterGuard<T> { std::forward<T>(awaitable) };
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

        auto await_suspend(BehaviorCoroutineHandle handle)
        {
            return mAwaiter.await_suspend(std::move(handle));
        }

        decltype(auto) await_resume()
        {
            return mAwaiter.await_resume();
        }

        Awaiter mAwaiter;        
    };

}
}
