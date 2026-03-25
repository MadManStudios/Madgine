#include "../moduleslib.h"

#include "taskpromise.h"

#include "taskqueue.h"

#if ENABLE_TASK_TRACKING
#    include "Interfaces/debug/stacktrace.h"
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

    bool TaskPromiseBase::set_done()
    {
        return false;
    }

    Execution::StopToken tag_invoke(Execution::get_stop_token_t, TaskPromiseBase &promise)
    {
        return promise.mQueue->stopToken();
    }

    bool TaskPromiseBase::immediate() const
    {
        return mImmediate;
    }

    void TaskInitialSuspend::await_resume() noexcept
    {
#if ENABLE_TASK_TRACKING
        Debug::Tasks::onAssign(std::coroutine_handle<TaskSuspendablePromiseTypeBase>::from_promise(*mPromise), mPromise->queue(), Debug::StackTrace<1>::getCurrent(1));
#endif
    }

#ifndef NDEBUG
    Debug::FullStackTrace TaskPromiseBase::getSuspensionPoint()
    {
        return mCurrentSuspensionPoint.calculateReadable();
    }
#endif

}
}