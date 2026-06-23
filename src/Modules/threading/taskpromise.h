#pragma once

#include "Generic/execution/awaitablesender.h"
#include "Generic/execution/concepts.h"

#include "taskhandle.h"
#include "taskpromisesharedstate.h"

#if MODULES_ENABLE_TASK_TRACKING
#    include "../debug/tasktracking/tasktracker.h"
#endif

namespace Engine {
namespace Threading {

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
        void await_resume(std::source_location location = std::source_location::current()) noexcept;

#if ENABLE_TASK_TRACKING
        TaskPromiseBase *mPromise;
#endif
    };

    struct MODULES_EXPORT TaskPromiseBase {
        TaskPromiseBase(bool immediate = false);
        ~TaskPromiseBase();

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
            throw;
        }

        void then_return(TaskHandle handle)
        {
            assert(!mThenReturn);
            mThenReturn = std::move(handle);
        }

        static auto unpack_storage(auto &&storage)
        {
            return std::forward<decltype(storage)>(storage);
        }

        template <typename T>
        decltype(auto) await_transform(T &&awaitable, std::source_location location = std::source_location::current())
        {
            mCurrentSuspensionPoint = std::move(location);

            if constexpr (Execution::AnySender<std::remove_reference_t<T>>) {
                return Execution::AwaitableSender<T, TaskPromiseBase, TaskHandle> { std::forward<T>(awaitable), *this };
            } else {
                return std::forward<T>(awaitable);
            }
        }

        void setQueue(TaskQueue *queue);
        TaskQueue *queue() const;

        friend MODULES_EXPORT Execution::StopToken tag_invoke(Execution::get_stop_token_t, TaskPromiseBase &promise);

        bool immediate() const;

        std::bool_constant<false> set_done();
        std::bool_constant<false> set_value(auto &&...)
        {
            return set_done();
        }
        std::bool_constant<false> set_error(auto &&)
        {
            return set_done();
        }

    protected:
        TaskHandle mThenReturn;

        std::shared_ptr<TaskPromiseSharedStateBase> mState;

    private:
        TaskQueue *mQueue = nullptr;
        bool mImmediate;

        std::source_location mCurrentSuspensionPoint;

    public:
        const std::source_location &getSuspensionPoint();
    };

    template <typename T>
    struct TaskPromise : TaskPromiseBase {

        using TaskPromiseBase::TaskPromiseBase;

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
    struct TaskPromise<void> : TaskPromiseBase {

        using TaskPromiseBase::TaskPromiseBase;

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