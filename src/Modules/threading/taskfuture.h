#pragma once

#include "taskhandle.h"
#include "taskpromisesharedstate.h"

namespace Engine {
namespace Threading {

    template <typename F, typename... Args>
    auto make_task(F f, Args &&...args);

    template <typename T>
    struct TaskFuture {

        TaskFuture() = default;

        TaskFuture(T val)
            : mState(std::make_shared<TaskPromiseSharedState<T>>(std::move(val)))
        {
        }

        TaskFuture(std::shared_ptr<TaskPromiseSharedState<T>> state)
            : mState(std::move(state))
        {
        }

        TaskFuture(const TaskFuture &) = default;
        TaskFuture(TaskFuture &&) = default;

        TaskFuture &operator=(const TaskFuture &) = default;
        TaskFuture &operator=(TaskFuture &&) = default;

        bool valid() const
        {
            return static_cast<bool>(mState);
        }

        bool attached() const
        {
            return mState && mState->mAttached;
        }

        void reset()
        {
            mState.reset();
        }

        std::shared_ptr<TaskPromiseSharedState<T>> release()
        {
            return std::move(mState);
        }

        bool is_ready() const
        {
            return mState->is_ready();
        }

        auto &sender() const 
        {
            return mState->sender();
        }

        auto operator co_await() const
        {
            return TaskFutureAwaitable<T> { mState };
        }

        template <typename U>
        ImmediateTask<U> then_task(Task<U> task)
        {
            return [](Task<U> task, TaskFuture<T> fut) -> ImmediateTask<U> {
                co_await fut;
                co_return co_await std::move(task);
            }(std::move(task), *this);
        }

        template <typename F>
        auto then(F &&f)
        {
            using WrappedTask = ImmediateTask<typename decltype(make_task(std::declval<F>(), std::declval<T>()))::T>;
            return [](F f, TaskFuture<T> fut) -> WrappedTask {
                const T &value = co_await fut;
                co_await make_task(std::forward<F>(f), value);
            }(std::forward<F>(f), *this);
        }

        const T &get() const
        {
            assert(is_ready());
            return mState->get();
        }

        operator const T &() const
        {
            return get();
        }

    private:
        std::shared_ptr<TaskPromiseSharedState<T>> mState;
    };

    template <>
    struct TaskFuture<void> {

        TaskFuture() = default;

        TaskFuture(std::shared_ptr<TaskPromiseSharedState<void>> state)
            : mState(std::move(state))
        {
        }

        static TaskFuture make_ready()
        {
            return { std::make_shared<TaskPromiseSharedState<void>>(true) };
        }

        bool valid() const
        {
            return static_cast<bool>(mState);
        }

        bool attached() const
        {
            return mState && mState->mAttached;
        }

        void reset()
        {
            mState.reset();
        }

        std::shared_ptr<TaskPromiseSharedState<void>> release()
        {
            return std::move(mState);
        }

        bool is_ready() const
        {
            return mState->is_ready();
        }

        auto sender() const
        {
            return mState->sender();
        }
           
        auto operator co_await() const
        {
            return TaskFutureAwaitable<void> { mState };
        }

        template <typename U>
        ImmediateTask<U> then_task(Task<U> task)
        {
            return [](Task<U> task, TaskFuture<void> fut) -> ImmediateTask<U> {
                co_await fut;
                co_return co_await std::move(task);
            }(std::move(task), *this);
        }

        template <typename F>
        auto then(F f)
        {
            using WrappedTask = ImmediateTask<typename decltype(make_task(std::declval<F>()))::T>;
            return [](F f, TaskFuture<void> fut) -> WrappedTask {
                co_await fut;
                co_await make_task(std::forward<F>(f));
            }(std::forward<F>(f), *this);
        }

    private:
        std::shared_ptr<TaskPromiseSharedState<void>> mState;
    };

    template <typename T>
    struct AtomicTaskFuture {

        TaskFuture<T> ensure()
        {
            std::shared_ptr<TaskPromiseSharedState<T>> check = mState.load();
            if (!check) {
                std::shared_ptr<TaskPromiseSharedState<T>> state = std::make_shared<TaskPromiseSharedState<T>>();
                if (mState.compare_exchange_strong(check, state))
                    return state;
            }
            return check;
        }

        TaskFuture<T> load() const
        {
            return mState.load();
        }

        void reset()
        {
            mState.store({});
        }

    private:
        std::atomic<std::shared_ptr<TaskPromiseSharedState<T>>> mState;
    };

}
}