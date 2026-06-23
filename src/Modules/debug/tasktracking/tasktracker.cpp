#include "../../moduleslib.h"

#include "tasktracker.h"

#include "../../threading/taskpromise.h"
#include "../../threading/taskqueue.h"

namespace Engine {
namespace Debug {

    namespace Tasks {

        void TaskTracker::onAssign(void *ident, std::source_location location)
        {
            std::lock_guard guard { mMutex };

            // mEvents.emplace_back(Event:: { stacktrace }, ident);
            auto pib = mTasksInFlight.try_emplace(ident, std::move(location));
            // assert(pib.second);
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

        void TaskTracker::onSuspend(void *ident, std::chrono::high_resolution_clock::time_point timePoint)
        {
            std::lock_guard guard { mMutex };
            mEvents.emplace_back(Event::SUSPEND, ident, timePoint);
        }

        void TaskTracker::onDestroy(void *ident)
        {
            // std::lock_guard guard { mMutex };
            // mEvents.emplace_back(Event::DESTROY, ident);
            // auto count = mTasksInFlight.erase(ident);
            // assert(count == 1);
        }

        const std::source_location &TaskTracker::getTraceback(void *ident)
        {
            std::lock_guard guard { mMutex };
            return mTasksInFlight.at(ident);
        }

        const std::deque<TaskTracker::Event> &TaskTracker::events() const
        {
            return mEvents;
        }

        const std::map<void *, std::source_location> TaskTracker::tasksInFlight() const
        {
            return mTasksInFlight;
        }

        void onAssign(const std::coroutine_handle<> &handle, Threading::TaskQueue *queue, std::source_location location)
        {
            queue->mTracker.onAssign(handle.address(), std::move(location));
        }

        void onEnter(const std::coroutine_handle<> &handle, Threading::TaskQueue *queue)
        {
            queue->mTracker.onEnter(handle.address());
        }

        void onReturn(const std::coroutine_handle<> &handle, Threading::TaskQueue *queue)
        {
            queue->mTracker.onReturn(handle.address());
        }

        std::pair<Threading::TaskQueue*, void *> onResume(const Threading::TaskHandle &handle)
        {
            handle.queue()->mTracker.onResume(handle.address());
            return { handle.queue(), handle.address() };
        }

        void onSuspend(std::pair<Threading::TaskQueue *, void *> data)
        {
            data.first->mTracker.onSuspend(data.second);
        }

        void onDestroy(Engine::Threading::TaskPromiseBase &promise)
        {
            promise.queue()->mTracker.onDestroy(std::coroutine_handle<Engine::Threading::TaskPromiseBase>::from_promise(promise).address());
        }

    }

}
}