#pragma once

#include "Generic/execution/algorithm.h"
#include "Generic/execution/sender.h"
#include "Generic/execution/storage.h"
#include "Generic/makeowning.h"
#include "Generic/withresult.h"

namespace Engine {
namespace Threading {

    template <typename Sender>
    struct TaskAwaitableSender;

    template <typename Sender>
    struct TaskAwaitableReceiver : Execution::execution_receiver<> {

        template <typename... V>
        void set_value(V &&...value)
        {
            mState->set_value(std::forward<V>(value)...);
        }

        void set_done()
        {
            mState->set_done();
        }

        template <typename... R>
        void set_error(R &&...result)
        {
            mState->set_error(std::forward<R>(result)...);
        }

        TaskAwaitableSender<Sender> *mState;
    };

    template <typename Sender>
    struct TaskAwaitableSender {

        using S = Execution::connect_result_t<Sender, TaskAwaitableReceiver<Sender>>;

        TaskAwaitableSender(Sender &&sender)
            : mState(Execution::connect(std::forward<Sender>(sender), TaskAwaitableReceiver<Sender> { {}, this }))
        {
        }

        bool await_ready()
        {
            mState.start();
            return mFlag.test();
        }

        bool await_suspend(TaskHandle task)
        {
            mTask = std::move(task);
            if (mFlag.test_and_set()) {
                mTask.release();
                return false;
            } else {
                return true;
            }
        }

        Execution::ResultStorage<Sender> await_resume()
        {
            return std::move(mResult);
        }

        template <typename... V>
        void set_value(V &&... v)
        {
            mResult.set_value(std::forward<V>(v)...);
            if (mFlag.test_and_set())
                mTask.resumeInQueue();
        }

        void set_done()
        {
            mResult.set_done();
            if (mFlag.test_and_set())
                mTask.resumeInQueue();
        }

        template <typename... R>
        void set_error(R&&... result)
        {
            mResult.set_error(std::forward<R>(result)...);
            if (mFlag.test_and_set())
                mTask.resumeInQueue();
        }

    private:
        S mState;
        std::atomic_flag mFlag;
        TaskHandle mTask;
        Execution::ResultStorage<Sender> mResult;
    };

}
}
