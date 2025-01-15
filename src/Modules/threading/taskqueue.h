#pragma once

#include "task.h"

#if ENABLE_TASK_TRACKING
#    include "../debug/tasktracking/tasktracker.h"
#endif

namespace Engine {
namespace Threading {

    struct MODULES_EXPORT TaskQualifiers {

        TaskQualifiers() = default;

        TaskQualifiers(std::chrono::steady_clock::time_point timepoint)
            : mScheduledFor(timepoint)
        {
        }

        TaskQualifiers(std::chrono::steady_clock::duration duration)
            : mScheduledFor(std::chrono::steady_clock::now() + duration)
        {
        }

        bool await_ready();
        void await_suspend(TaskHandle handle);
        void await_resume();

        std::chrono::steady_clock::time_point mScheduledFor = std::chrono::steady_clock::now();
    };

    struct MODULES_EXPORT TaskQueue {

        TaskQueue(const std::string &name, bool wantsMainThread = false);
        ~TaskQueue();

#if !EMSCRIPTEN
        void update(int taskCount = -1);
        void notify();
#endif

        template <typename T, bool Immediate>
        TaskFuture<T> queueTask(Task<T, Immediate> task, TaskQualifiers qualifiers = {})
        {
            TaskFuture<T> fut = task.get_future();
            TaskHandle handle = task.assign(this);
            if (handle)
                queueHandle(std::move(handle), false, std::move(qualifiers));
            return fut;
        }

        template <typename F>
        void queue(F &&f, TaskQualifiers qualifiers = {})
        {
            TaskHandle handle = make_task(std::forward<F>(f)).assign(this);
            if (handle)
                queueHandle(std::move(handle), false, std::move(qualifiers));
        }

        void increaseTaskInFlightCount();
        void decreaseTaskInFlightCount();
        size_t taskInFlightCount() const;

        bool idle() const;

        const std::string &name() const;

        bool running() const;
        void stop();

        bool wantsMainThread() const;

        template <typename Init, typename Finalize>
        void addSetupSteps(Init &&init, Finalize &&finalize)
        {
            auto initTask = make_task(std::forward<Init>(init));
            auto future = initTask.get_future();
            auto finalizeTask = make_task(LIFT(TupleUnpacker::invoke), std::forward<Finalize>(finalize), std::move(future));
            addSetupStepTasks(std::move(initTask), std::move(finalizeTask));
        }

        template <typename Init>
        void addSetupSteps(Init &&init)
        {
            auto initTask = make_task(std::forward<Init>(init));
            addSetupStepTasks(std::move(initTask));
        }

#if ENABLE_TASK_TRACKING
        Debug::Tasks::TaskTracker mTracker;
#endif

    protected:
        struct ScheduledTask {
            TaskHandle mTask;
            TaskQualifiers mQualifiers;
        };

        friend struct TaskHandle;
        friend struct TaskQualifiers;

        void queueHandle(TaskHandle task, bool resume, TaskQualifiers qualifiers = {});

        void queueInternal(ScheduledTask tracker, bool resume);

        void addSetupStepTasks(Task<bool> init, Task<void> finalize = {});

        Task<bool> patchSetupTask(Task<bool> init);

#if !EMSCRIPTEN
        TaskHandle fetch();
#endif

    private:
        std::string mName;
        bool mWantsMainThread;

        WorkGroup &mWorkGroup;

        std::atomic<size_t> mTaskInFlightCount = 0;
        std::atomic<size_t> mSetupTaskInFlightCount = 0;
        bool mInitializing = true;

        std::list<ScheduledTask> mBacklog;

        std::list<Task<void>> mCleanupSteps;

#if !EMSCRIPTEN
        std::list<ScheduledTask> mQueue;
        std::condition_variable mCv;
#endif

        mutable std::mutex mMutex;
    };

}
}