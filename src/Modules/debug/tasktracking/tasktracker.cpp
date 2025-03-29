#include "../../moduleslib.h"

#include "tasktracker.h"

#include "../../threading/taskpromise.h"
#include "../../threading/taskqueue.h"

namespace Engine {
namespace Debug {

    namespace Tasks {

        void TaskTracker::onAssign(void *ident, StackTrace<1> stacktrace)
        {
            std::lock_guard guard { mMutex };

            //mEvents.emplace_back(Event:: { stacktrace }, ident);
            auto pib = mTasksInFlight.try_emplace(ident, stacktrace);
            //assert(pib.second);
        }

        void TaskTracker::onEnter(void *ident, std::chrono::high_resolution_clock::time_point timePoint)
        {
            std::lock_guard guard { mMutex };
            mEvents.emplace_back(Event::ENTER, ident, timePoint);
        }

        void TaskTracker::onReturn(void *ident, std::chrono::high_resolution_clock::time_point timePoint)
        {
            std::lock_guard guard { mMutex };
            mEvents.emplace_back(Event::RETURN, ident, timePoint);
        }

        void TaskTracker::onResume(void *ident, std::chrono::high_resolution_clock::time_point timePoint)
        {
            std::lock_guard guard { mMutex };
            mEvents.emplace_back(Event::RESUME, ident, timePoint);
        }

        void TaskTracker::onSuspend(std::chrono::high_resolution_clock::time_point timePoint)
        {
            std::lock_guard guard { mMutex };
            mEvents.emplace_back(Event::SUSPEND, nullptr, timePoint);
        }

        void TaskTracker::onDestroy(void *ident)
        {
            //std::lock_guard guard { mMutex };
            //mEvents.emplace_back(Event::DESTROY, ident);
            //auto count = mTasksInFlight.erase(ident);
            //assert(count == 1);
        }

        TraceBack TaskTracker::getTraceback(void *ident)
        {
            std::lock_guard guard { mMutex };
            FullStackTrace trace = mTasksInFlight.at(ident).calculateReadable();
            if (!trace.empty())
                return trace.front();
            else
                return {};
        }

        const std::deque<TaskTracker::Event> &TaskTracker::events() const
        {
            return mEvents;
        }

        const std::map<void *, StackTrace<1>> TaskTracker::tasksInFlight() const
        {
            return mTasksInFlight;
        }

        void onAssign(const std::coroutine_handle<> &handle, Engine::Threading::TaskQueue *queue, StackTrace<1> stacktrace)
        {
            queue->mTracker.onAssign(handle.address(), stacktrace);
        }

        void onEnter(const std::coroutine_handle<> &handle, Engine::Threading::TaskQueue *queue)
        {
            queue->mTracker.onEnter(handle.address());
        }

        void onReturn(const std::coroutine_handle<> &handle, Engine::Threading::TaskQueue *queue)
        {
            queue->mTracker.onReturn(handle.address());
        }

        void onResume(const Engine::Threading::TaskHandle &handle)
        {
            handle.queue()->mTracker.onResume(handle.address());
        }

        void onSuspend(Engine::Threading::TaskQueue *queue)
        {
            queue->mTracker.onSuspend();
        }

        void onDestroy(Engine::Threading::TaskSuspendablePromiseTypeBase &promise)
        {
            promise.queue()->mTracker.onDestroy(std::coroutine_handle<Engine::Threading::TaskSuspendablePromiseTypeBase>::from_promise(promise).address());
        }

    }

}
}