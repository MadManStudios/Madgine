#include "../moduleslib.h"

#include "taskhandle.h"

#include "taskpromise.h"
#include "taskqueue.h"

#if MODULES_ENABLE_TASK_TRACKING
#    include "../debug/tasktracking/tasktracker.h"
#endif

namespace Engine {
namespace Threading {

    TaskHandle::TaskHandle(Execution::CoroutineHandle<TaskPromiseBase> handle)
        : mHandle(handle.release())
    {
    }

    TaskHandle::TaskHandle(TaskHandle &&other)
        : mHandle(std::exchange(other.mHandle, {}))
    {
    }

    TaskHandle &TaskHandle::operator=(TaskHandle &&other)
    {
        std::swap(mHandle, other.mHandle);
        return *this;
    }

    TaskHandle::~TaskHandle()
    {
        assert(!mHandle);
    }

    void TaskHandle::operator()()
    {
#if MODULES_ENABLE_TASK_TRACKING
        auto id = Debug::Tasks::onResume(*this);
#endif
        // Reset mHandle to allow exception handling
        std::coroutine_handle<TaskPromiseBase> handle = mHandle;
        mHandle = {};
        handle.resume();
#if MODULES_ENABLE_TASK_TRACKING
        Debug::Tasks::onSuspend(id);
#endif
    }

    void TaskHandle::resume()
    {
        queue()->queueHandle(std::move(*this), true);
    }

    std::coroutine_handle<TaskPromiseBase> TaskHandle::release()
    {
        return std::exchange(mHandle, std::coroutine_handle<TaskPromiseBase> {});
    }

    TaskPromiseBase &TaskHandle::promise()
    {
        return mHandle.promise();
    }

    TaskQueue *TaskHandle::queue() const
    {
        return mHandle.promise().queue();
    }

    void *TaskHandle::address() const
    {
        return mHandle.address();
    }

    TaskHandle::operator bool() const
    {
        return static_cast<bool>(mHandle);
    }

}
}