#include "../moduleslib.h"

#include "taskpromise.h"
#include "taskqueue.h"

#if ENABLE_TASK_TRACKING
#    include "Interfaces/debug/stacktrace.h"
#endif

namespace Engine {
namespace Threading {

    TaskSuspendablePromiseTypeBase::TaskSuspendablePromiseTypeBase(bool immediate)
        : mImmediate(immediate)
    {
    }

    TaskSuspendablePromiseTypeBase::~TaskSuspendablePromiseTypeBase()
    {
        if (mQueue) {
            mQueue->decreaseTaskInFlightCount();
#if ENABLE_TASK_TRACKING
            Debug::Tasks::onDestroy(*this);
#endif
        }
    }

    void TaskSuspendablePromiseTypeBase::setQueue(TaskQueue *queue)
    {
        assert(queue);
        assert(!mQueue);
        mQueue = queue;
        queue->increaseTaskInFlightCount();
    }

    TaskQueue *TaskSuspendablePromiseTypeBase::queue() const
    {
        return mQueue;
    }

    bool TaskSuspendablePromiseTypeBase::immediate() const
    {
        return mImmediate;
    }

    void TaskInitialSuspend::await_resume() noexcept
    {
#if ENABLE_TASK_TRACKING
        Debug::Tasks::onAssign(std::coroutine_handle<TaskSuspendablePromiseTypeBase>::from_promise(*mPromise), mPromise->queue(), Debug::StackTrace<1>::getCurrent(1));
#endif
    }

}
}