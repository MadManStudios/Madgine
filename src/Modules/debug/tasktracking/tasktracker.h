#pragma once

#include "Interfaces/debug/stacktrace.h"

namespace Engine {
namespace Debug {

    namespace Tasks {

        MODULES_EXPORT void onAssign(const std::coroutine_handle<> &handle, Engine::Threading::TaskQueue *queue, StackTrace<1> stacktrace);
        MODULES_EXPORT void onDestroy(Engine::Threading::TaskSuspendablePromiseTypeBase &promise);

        MODULES_EXPORT void onResume(const Engine::Threading::TaskHandle &handle);
        MODULES_EXPORT void onSuspend(Engine::Threading::TaskQueue *queue);

        MODULES_EXPORT void onEnter(const std::coroutine_handle<> &handle, Engine::Threading::TaskQueue *queue);
        MODULES_EXPORT void onReturn(const std::coroutine_handle<> &handle, Engine::Threading::TaskQueue *queue);

        struct MODULES_EXPORT TaskTracker {

            void onAssign(void *ident, StackTrace<1> stacktrace);
            void onEnter(void *ident, std::chrono::high_resolution_clock::time_point timePoint = std::chrono::high_resolution_clock::now());
            void onReturn(void *ident, std::chrono::high_resolution_clock::time_point timePoint = std::chrono::high_resolution_clock::now());
            void onResume(void *ident, std::chrono::high_resolution_clock::time_point timePoint = std::chrono::high_resolution_clock::now());
            void onSuspend(std::chrono::high_resolution_clock::time_point timePoint = std::chrono::high_resolution_clock::now());
            void onDestroy(void *ident);

            TraceBack getTraceback(void *ident);

            struct Event {
                enum Type {
                    ENTER,
                    RETURN,
                    RESUME,
                    SUSPEND
                } mType;
                void *mIdentifier;
                std::chrono::high_resolution_clock::time_point mTimePoint = std::chrono::high_resolution_clock::now();

                Event(Type type, void *ident = nullptr, std::chrono::high_resolution_clock::time_point timePoint = std::chrono::high_resolution_clock::now())
                    : mType(type)
                    , mIdentifier(ident)
                    , mTimePoint(timePoint)
                {
                }
            };

            const std::deque<Event> &events() const;
            const std::map<void *, StackTrace<1>> tasksInFlight() const;

            std::mutex mMutex;

            std::thread::id mThread;

        private:
            std::deque<Event> mEvents;
            std::map<void *, StackTrace<1>> mTasksInFlight;
        };

    }

}
}