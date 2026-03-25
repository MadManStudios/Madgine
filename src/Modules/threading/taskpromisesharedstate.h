#pragma once

#include "Generic/delayedconstruct.h"
#include "Generic/execution/flag.h"

#include "taskhandle.h"

namespace Engine {
namespace Threading {

    struct MODULES_EXPORT TaskPromiseSharedStateBase {

        TaskPromiseSharedStateBase(bool ready = false);
        ~TaskPromiseSharedStateBase();

        std::mutex mMutex;

        bool mAttached;
        bool mDestroyed;
        bool mDone;

        void attach();
        void finalize();

        void notifyDestroyed();
    };

    template <typename T>
    struct TaskPromiseSharedState : TaskPromiseSharedStateBase {

        TaskPromiseSharedState() = default;
        TaskPromiseSharedState(T val)
            : TaskPromiseSharedStateBase(true)
        {
            mFlag.set_value(std::forward<T>(val));
        }

        Execution::Flag<void, T> mFlag;

        bool valid()
        {
            std::lock_guard guard { mMutex };
            return mFlag.isSet() || !mDestroyed;
        }

        bool is_ready()
        {
            std::lock_guard guard { mMutex };
            return mFlag.isSet();
        }

        void set_value(T val)
        {
            ManualLifetime<typename Execution::Flag<void, T>::CallbackDelay> cb;
            {
                std::lock_guard guard { mMutex };
                construct(cb, mFlag.set_value(std::move(val)));
            }
            destruct(cb);
        }

        const T &get()
        {
            std::lock_guard guard { mMutex };
            assert(mFlag.isSet());
            return *mFlag;
        }

        auto &sender()
        {
            return mFlag;
        }
    };

    template <>
    struct TaskPromiseSharedState<void> : TaskPromiseSharedStateBase {

        TaskPromiseSharedState(bool ready = false)
            : TaskPromiseSharedStateBase(ready)
        {
            if (ready)
                mFlag.set_value();
        }

        Execution::Flag<void> mFlag;

        bool valid()
        {
            std::lock_guard guard { mMutex };
            return mFlag.isSet() || !mDestroyed;
        }

        bool is_ready()
        {
            std::lock_guard guard { mMutex };
            return mFlag.isSet();
        }

        void set_value()
        {
            {
                std::lock_guard guard { mMutex };
                mFlag.set_value();
            }
        }

        void get()
        {
            std::lock_guard guard { mMutex };
            assert(mFlag.isSet());
        }

        auto &sender()
        {
            return mFlag;
        }
    };

    template <typename T>
    struct TaskFutureAwaitable;

    template <typename T>
    struct TaskFutureAwaitableReceiver {

        template <typename... V>
        void set_value(V &&...value)
        {
            mState->set_value();
        }

        void set_done()
        {
            throw 0;
        }

        template <typename... E>
        void set_error(E &&...)
        {
            throw 0;
        }

        TaskFutureAwaitable<T> *mState;
    };

    template <typename T>
    struct TaskFutureAwaitable {

        using Sender = std::invoke_result_t<decltype(&TaskPromiseSharedState<T>::sender), TaskPromiseSharedState<T>>;

        using S = Execution::connect_result_t<Sender, TaskFutureAwaitableReceiver<T>>;

        TaskFutureAwaitable(std::shared_ptr<TaskPromiseSharedState<T>> promiseState)
            : mPromiseState(std::move(promiseState))
        {
        }

        bool await_ready()
        {
            return mPromiseState->is_ready();
        }

        bool await_suspend(TaskHandle task)
        {
            mTask = std::move(task);
            construct(mState, DelayedConstruct<S> { [this]() { return Execution::connect(mPromiseState->sender(), TaskFutureAwaitableReceiver<T> { this }); } });
            mState->start();

            if (mFlag.test_and_set()) {
                mTask.release();
                return false;
            } else {
                return true;
            }
        }

        decltype(auto) await_resume()
        {
            return mPromiseState->get();
        }

        void set_value()
        {
            destruct(mState);
            if (mFlag.test_and_set())
                mTask.resume();
        }

    private:
        ManualLifetime<S> mState;
        std::atomic_flag mFlag;
        TaskHandle mTask;
        std::shared_ptr<TaskPromiseSharedState<T>> mPromiseState;
    };

}
}