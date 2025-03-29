#pragma once

#include "Generic/coroutines/handle.h"
#include "taskpromise.h"

namespace Engine {
namespace Threading {

    template <typename>
    struct is_task : std::false_type {
    };

    template <typename T, bool I>
    struct is_task<Task<T, I>> : std::true_type {
    };

    template <typename T>
    concept IsTask = is_task<T>::value;

    template <typename _T, bool Immediate>
    struct [[nodiscard]] Task {

        using T = _T;

        static_assert(!IsTask<T>);

        struct promise_type : TaskSuspendablePromise<T> {

            promise_type()
                : TaskSuspendablePromise<T>(Immediate)
            {
            }

            Task<T, Immediate> get_return_object()
            {
                return { CoroutineHandle<promise_type>::fromPromise(*this) };
            }
        };

        template <typename T2, bool I>
        friend struct Task;

        Task() = default;

        template <bool I>
        Task(Task<T, I> &&other)
            : mHandle(std::move(other.mHandle))
            , mState(std::move(other.mState))
        {
        }

        Task(std::shared_ptr<TaskPromiseSharedState<T>> state)
            : mState(std::move(state))
        {
        }

        Task(CoroutineHandle<promise_type> handle)
            : mHandle(std::move(handle))
        {
        }

        TaskFuture<T> get_future()
        {
            if (!mState) {
                mState = mHandle->get_state();
            }
            return mState;
        }

        void set_future(TaskFuture<T> fut)
        {
            assert(!mState && fut.valid());
            mState = fut.release();
            mHandle->set_state(mState);
        }

        TaskHandle assign(TaskQueue *queue)
        {
            TaskHandle handle;
            if (mHandle) {
                mHandle->setQueue(queue);
                bool immediate = mHandle->immediate();
                handle = { std::move(mHandle) };
                if (immediate)
                    handle();
            }
            return handle;
        }

        bool await_ready() noexcept { return !mHandle; }
        std::coroutine_handle<> await_suspend(TaskHandle handle) noexcept
        {
            assert(mHandle);
            if (!mState) {
                mState = mHandle->get_state();
            }
            mHandle->setQueue(handle.queue());
            mHandle->then_return(std::move(handle));
#if MODULES_ENABLE_TASK_TRACKING
            Debug::Tasks::onEnter(mHandle.get(), mHandle->queue());
#endif
            return mHandle.get();
        }
        T await_resume() noexcept
        {
            assert(!mHandle || mHandle.done());
            return mState->get();
        }

        template <typename F>
        Task<std::invoke_result_t<F>, Immediate> then(F&& f) &&{
            return [](F f, Task task) -> Task<std::invoke_result_t<F>, Immediate>{
                co_await std::move(task);
                f();
            }(std::forward<F>(f), std::move(*this));
        }

    private:
        CoroutineHandle<TaskSuspendablePromise<T>> mHandle;
        std::shared_ptr<TaskPromiseSharedState<T>> mState;
    };

    template <typename T>
    using ImmediateTask = Task<T, true>;

    template <typename T>
    Task<T> make_ready_task(T &&val)
    {
        return { std::make_shared<TaskPromiseSharedState<T>>(std::forward<T>(val)) };
    }

    template <typename F, typename... Args>
    auto make_task(F f, Args &&...args)
    {
        static_assert(!std::is_reference_v<F>);

        using R = std::invoke_result_t<F, Args...>;

        if constexpr (IsTask<R>) {
            if constexpr (sizeof(F) == 1) { //most likely captureless lambda
                return std::invoke(std::move(f), std::forward<Args>(args)...);
            } else {
                return [](F f, Args... args) -> R { //keep f alive during the whole lifetime of the Task
                    co_return co_await std::invoke(std::move(f), std::forward<Args>(args)...);
                }(std::move(f), std::forward<Args>(args)...);
            }
        } else {
            return [](F f, Args... args) -> Task<R> {
                co_return std::invoke(std::move(f), std::forward<Args>(args)...);
            }(std::move(f), std::forward<Args>(args)...);
        }
    }

}
}