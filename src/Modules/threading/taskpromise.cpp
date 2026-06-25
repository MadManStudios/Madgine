#include "../moduleslib.h"

#include "taskpromise.h"

#include "taskqueue.h"

#if ENABLE_TASK_TRACKING
#    include "Platform/debug/stacktrace.h"
#endif

namespace Engine {
namespace Threading {

    TaskPromiseBase::TaskPromiseBase(bool immediate)
        : mImmediate(immediate)
    {
    }

    TaskPromiseBase::~TaskPromiseBase()
    {
        if (mQueue) {
            mQueue->unregisterTaskInFlight(this);
#if ENABLE_TASK_TRACKING
            Debug::Tasks::onDestroy(*this);
#endif
        }
        if (mState)
            mState->notifyDestroyed();
    }

    void TaskPromiseBase::setQueue(TaskQueue *queue)
    {
        assert(queue);
        assert(!mQueue);
        mQueue = queue;
        queue->registerTaskInFlight(this);
    }

    TaskQueue *TaskPromiseBase::queue() const
    {
        return mQueue;
    }

    std::bool_constant<false> TaskPromiseBase::set_done()
    {
        return {};
    }

    Execution::StopToken tag_invoke(Execution::get_stop_token_t, TaskPromiseBase &promise)
    {
        return promise.mQueue->stopToken();
    }

    bool TaskPromiseBase::immediate() const
    {
        return mImmediate;
    }

    void TaskInitialSuspend::await_resume(std::source_location location) noexcept
    {
#if ENABLE_TASK_TRACKING
        Debug::Tasks::onAssign(std::coroutine_handle<TaskPromiseBase>::from_promise(*mPromise), mPromise->queue(), std::move(location));
        if (!mPromise->mThenReturn)
            Debug::Tasks::onResume(mPromise->queue(), std::coroutine_handle<TaskPromiseBase>::from_promise(*mPromise).address(), 0);
#endif
    }

    bool TaskFinalSuspend::await_ready() noexcept
    {
#if MODULES_ENABLE_TASK_TRACKING
        if (mHandle) {
            Debug::Tasks::onReturn(mHandle.address(), mPromise->queue());
        } else {
            uint16_t depth = Debug::Tasks::onSuspend(mPromise->queue(), nullptr);
            assert(depth == 0);
        }
#endif
        return !mHandle;
    }

    std::coroutine_handle<> TaskFinalSuspend::await_suspend(std::coroutine_handle<> self) noexcept
    {
        assert(mHandle);
        return mHandle.release();
    }

    const std::source_location &TaskPromiseBase::getSuspensionPoint()
    {
        return mCurrentSuspensionPoint;
    }

}
}