#pragma once

#include "Generic/execution/algorithm.h"
#include "Generic/execution/make_sender.h"
#include "Generic/execution/stoppable.h"
#include "Generic/execution/storage.h"

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

        friend auto tag_invoke(Execution::get_stop_token_t, TaskAwaitableReceiver &rec)
        {
            return rec.mStopToken;
        }

        TaskAwaitableSender<Sender> *mState;
        Execution::StopToken mStopToken;
    };

    template <typename Sender>
    struct TaskAwaitableSender {

        using S = Execution::connect_result_t<Execution::stoppable_t::sender<Sender>, TaskAwaitableReceiver<Sender>>;

        TaskAwaitableSender(Sender &&sender, Execution::StopToken stopToken)
            : mState(Execution::connect(std::forward<Sender>(sender) | Execution::stoppable, TaskAwaitableReceiver<Sender> { {}, this, stopToken }))
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
        void set_value(V &&...v)
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
        void set_error(R &&...result)
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
