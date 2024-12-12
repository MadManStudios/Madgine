#include "../moduleslib.h"

#include "taskqueue.h"
#include "workgroup.h"

#if EMSCRIPTEN
#    include <emscripten.h>
#endif

namespace Engine {
namespace Threading {

#if EMSCRIPTEN

    void emscripten_resume(void *address)
    {
        std::coroutine_handle<TaskSuspendablePromiseTypeBase>::from_address(address).resume();
    }

#endif

    bool TaskQualifiers::await_ready()
    {
        return false;
    }

    void TaskQualifiers::await_suspend(TaskHandle handle)
    {
        handle.queue()->queueHandle(std::move(handle), true, std::move(*this));
    }

    void TaskQualifiers::await_resume()
    {
    }

    TaskQueue::TaskQueue(const std::string &name, bool wantsMainThread)
        : mName(name)
        , mWantsMainThread(wantsMainThread)
        , mWorkGroup(WorkGroup::self())
    {
        mWorkGroup.addTaskQueue(this);
    }

    TaskQueue::~TaskQueue()
    {
#if !EMSCRIPTEN
        assert(mQueue.empty());
#endif
        assert(mTaskInFlightCount == 0);
        mWorkGroup.removeTaskQueue(this);
    }

    bool TaskQueue::wantsMainThread() const
    {
        return mWantsMainThread;
    }

    void TaskQueue::queueInternal(ScheduledTask task, bool resume)
    {
        assert(mWorkGroup.state() != WorkGroupState::DONE);
        //TODO: priority Queue
        std::lock_guard<std::mutex> lock(mMutex);
        if (resume || !mInitializing) {
#if !EMSCRIPTEN
            mQueue.emplace_back(std::move(task));
            mCv.notify_one();
#else
            emscripten_async_call(
                emscripten_resume,
                task.mTask.release().address(),
                std::chrono::duration_cast<std::chrono::milliseconds>(task.mQualifiers.mScheduledFor - std::chrono::steady_clock::now()).count() - 1);
#endif
        } else
            mBacklog.emplace_back(std::move(task));
    }

    const std::string &TaskQueue::name() const
    {
        return mName;
    }

#if !EMSCRIPTEN
    void TaskQueue::update(int taskCount)
    {
        while (TaskHandle task = fetch()) {
            task();
            if (taskCount > 0) {
                --taskCount;
                if (taskCount == 0) {
                    return;
                }
            }
        }
    }

    void TaskQueue::notify()
    {
        mCv.notify_all();
    }

#endif

    bool TaskQueue::running() const
    {
        return mWorkGroup.state() == WorkGroupState::RUNNING || (mWorkGroup.state() == WorkGroupState::INITIALIZING && !mInitializing);
    }

    void TaskQueue::stop()
    {
        mWorkGroup.stop();
    }

    void TaskQueue::queueHandle(TaskHandle task, bool resume, TaskQualifiers qualifiers)
    {
        queueInternal({ std::move(task), std::move(qualifiers) }, resume);
    }

    void TaskQueue::increaseTaskInFlightCount()
    {
        ++mTaskInFlightCount;
    }

    void TaskQueue::decreaseTaskInFlightCount()
    {
        --mTaskInFlightCount;
    }

    size_t TaskQueue::taskInFlightCount() const
    {
        return mTaskInFlightCount;
    }

#if !EMSCRIPTEN
    TaskHandle TaskQueue::fetch()
    {
        WorkGroupState state = mWorkGroup.state();
        if (state != WorkGroupState::DONE) {
            {
                std::unique_lock<std::mutex> lock(mMutex);
                while (!mQueue.empty()) {
                    auto now = std::chrono::steady_clock::now();
                    auto nextTaskTimepoint = std::chrono::steady_clock::time_point::max();
                    for (auto it = mQueue.begin(); it != mQueue.end(); ++it) {
                        if (it->mQualifiers.mScheduledFor <= now || state == WorkGroupState::STOPPING) {
                            TaskHandle task = std::move(it->mTask);
                            mQueue.erase(it);
                            return task;
                        } else {
                            nextTaskTimepoint = std::min(it->mQualifiers.mScheduledFor, nextTaskTimepoint);
                        }
                    }
                    mCv.wait_until(lock, nextTaskTimepoint);
                }
            }

            if (state == WorkGroupState::FINALIZING) {
                while (!mCleanupSteps.empty()) {
                    TaskHandle finalize = mCleanupSteps.back().assign(this);
                    mCleanupSteps.pop_back();
                    if (finalize) {
                        return finalize;
                    }
                }
            }
        }

        return {};
    }
#endif

    bool TaskQueue::idle() const
    {
        switch (mWorkGroup.state()) {
        case WorkGroupState::INITIALIZING:
            return !mInitializing;
        case WorkGroupState::FINALIZING:
            return mCleanupSteps.empty() && mTaskInFlightCount == 0;
        default:
            return mTaskInFlightCount == 0;
        }
    }

    Task<bool> TaskQueue::patchSetupTask(Task<bool> init)
    {
        bool result = co_await std::move(init);
        assert(mInitializing);
        if (--mSetupTaskInFlightCount == 0) {
            std::unique_lock lock { mMutex };
            mInitializing = false;
#if !EMSCRIPTEN
            assert(mQueue.empty());
            mQueue = std::move(mBacklog);
#else
            for (ScheduledTask &task : mBacklog) {
                emscripten_async_call(
                    emscripten_resume,
                    task.mTask.release().address(),
                    std::chrono::duration_cast<std::chrono::milliseconds>(task.mQualifiers.mScheduledFor - std::chrono::steady_clock::now()).count() - 1);
            }
#endif
            mBacklog.clear();
        }
        co_return result;
    }

    void TaskQueue::addSetupStepTasks(Task<bool> init, Task<void> finalize)
    {
        assert(mWorkGroup.state() == WorkGroupState::INITIALIZING);
        assert(mInitializing);

        ++mSetupTaskInFlightCount;
        TaskHandle initHandle = patchSetupTask(std::move(init)).assign(this);
        if (initHandle) {
#if !EMSCRIPTEN
            mQueue.push_back({ std::move(initHandle) });
#else
            emscripten_async_call(
                emscripten_resume,
                initHandle.release().address(),
                0);
#endif
        }

        mCleanupSteps.emplace_back(std::move(finalize));
    }

}
}