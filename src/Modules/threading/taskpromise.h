#pragma once

#include "taskfuture.h"
#include "taskhandle.h"

#include "Generic/execution/concepts.h"

#if MODULES_ENABLE_TASK_TRACKING
#    include "../debug/tasktracking/tasktracker.h"
#endif

namespace Engine {
namespace Threading {

    template <typename Sender>
    struct TaskAwaitableSender;

    struct TaskFinalSuspend {
        bool await_ready() noexcept { return !mHandle; }
        std::coroutine_handle<> await_suspend(std::coroutine_handle<> self) noexcept
        {
            assert(mHandle);
#if MODULES_ENABLE_TASK_TRACKING
            Debug::Tasks::onReturn(self, mHandle.queue());
#endif
            return mHandle.release();
        }
        void await_resume() noexcept { }

        TaskHandle mHandle;
    };

    struct MODULES_EXPORT TaskInitialSuspend {
        bool await_ready() noexcept { return false; }
        template <typename T>
        void await_suspend(std::coroutine_handle<T> self) noexcept
        {
#if ENABLE_TASK_TRACKING
            mPromise = &self.promise();
#endif
        }
        void await_resume() noexcept;

#if ENABLE_TASK_TRACKING
        TaskSuspendablePromiseTypeBase *mPromise;
#endif
    };

    struct MODULES_EXPORT TaskPromiseTypeBase {
        TaskPromiseTypeBase() = default;
        TaskPromiseTypeBase(const TaskPromiseTypeBase &) = delete;
        TaskPromiseTypeBase(TaskPromiseTypeBase &&) = default;
        TaskPromiseTypeBase &operator=(TaskPromiseTypeBase &&) = default;
        ~TaskPromiseTypeBase()
        {
            if (mState)
                mState->notifyDestroyed();
        }

    protected:
        std::shared_ptr<TaskPromiseSharedStateBase> mState;
    };

    struct MODULES_EXPORT TaskSuspendablePromiseTypeBase : TaskPromiseTypeBase {
        TaskSuspendablePromiseTypeBase(bool immediate = false);
        ~TaskSuspendablePromiseTypeBase();

        TaskInitialSuspend initial_suspend() noexcept
        {
            return {};
        }

        TaskFinalSuspend final_suspend() noexcept
        {
            if (mState)
                mState->finalize();
            return { std::move(mThenReturn) };
        }

        void unhandled_exception()
        {
            if (mState)
                mState->setException(std::current_exception());
            else
                throw;
        }

        void then_return(TaskHandle handle)
        {
            assert(!mThenReturn);
            mThenReturn = std::move(handle);
        }

        template <typename T>
        decltype(auto) await_transform(T &&awaitable)
        {
            if constexpr (Execution::Sender<std::remove_reference_t<T>>) {
                return TaskAwaitableSender<T> { std::forward<T>(awaitable) };
            } else {
                return std::forward<T>(awaitable);
            }
        }

        void setQueue(TaskQueue *queue);
        TaskQueue *queue() const;

        bool immediate() const;

    protected:
        TaskHandle mThenReturn;

    private:
        TaskQueue *mQueue = nullptr;
        bool mImmediate;
    };

    template <typename T>
    struct TaskPromise : TaskPromiseTypeBase {

        void set_value(T value)
        {
            if (mState) {
                static_cast<TaskPromiseSharedState<T> *>(mState.get())->set_value(std::move(value));
                mState->finalize();
            }
        }

        TaskFuture<T> get_future()
        {
            assert(!mState);
            std::shared_ptr<TaskPromiseSharedState<T>> state = std::make_shared<TaskPromiseSharedState<T>>();
            state->attach();
            mState = state;
            return state;
        }

        void set_future(TaskFuture<T> future)
        {
            assert(!mState && future.valid());
            mState = future.release();
            mState->attach();
        }
    };

    template <typename T>
    struct TaskSuspendablePromise : TaskSuspendablePromiseTypeBase {

        using TaskSuspendablePromiseTypeBase::TaskSuspendablePromiseTypeBase;

        void return_value(T value) noexcept
        {
            if (mState)
                static_cast<TaskPromiseSharedState<T> *>(mState.get())->set_value(std::move(value));
        }

        std::shared_ptr<TaskPromiseSharedState<T>> get_state()
        {
            assert(!mState);
            std::shared_ptr<TaskPromiseSharedState<T>> state = std::make_shared<TaskPromiseSharedState<T>>();
            state->attach();
            mState = state;
            return state;
        }

        void set_state(std::shared_ptr<TaskPromiseSharedState<T>> state)
        {
            assert(!mState && state);
            state->attach();
            mState = std::move(state);
        }
    };

    template <>
    struct TaskPromise<void> : TaskPromiseTypeBase {

        void set_value()
        {
            if (mState) {
                static_cast<TaskPromiseSharedState<void> *>(mState.get())->set_value();
                mState->finalize();
            }
        }

        TaskFuture<void> get_future()
        {
            assert(!mState);
            std::shared_ptr<TaskPromiseSharedState<void>> state = std::make_shared<TaskPromiseSharedState<void>>();
            state->attach();
            mState = state;
            return state;
        }

        void set_future(TaskFuture<void> future)
        {
            assert(!mState && future.valid());
            mState = future.release();
            mState->attach();
        }
    };

    template <>
    struct TaskSuspendablePromise<void> : TaskSuspendablePromiseTypeBase {

        using TaskSuspendablePromiseTypeBase::TaskSuspendablePromiseTypeBase;

        void return_void() noexcept
        {
            if (mState)
                static_cast<TaskPromiseSharedState<void> *>(mState.get())->set_value();
        }

        std::shared_ptr<TaskPromiseSharedState<void>> get_state()
        {
            assert(!mState);
            std::shared_ptr<TaskPromiseSharedState<void>> state = std::make_shared<TaskPromiseSharedState<void>>();
            state->attach();
            mState = state;
            return state;
        }

        void set_state(std::shared_ptr<TaskPromiseSharedState<void>> state)
        {
            assert(!mState && state);
            state->attach();
            mState = std::move(state);
        }
    };

}
}