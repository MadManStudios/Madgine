#pragma once

#include "Platform/debug/stacktrace.h"

namespace Engine {
namespace Debug {

    namespace Tasks {

        MODULES_EXPORT void onAssign(const std::coroutine_handle<> &handle, Engine::Threading::TaskQueue *queue, std::source_location location);
        MODULES_EXPORT void onDestroy(Engine::Threading::TaskPromiseBase &promise);

        MODULES_EXPORT std::pair<Threading::TaskQueue *, void *> onResume(const Engine::Threading::TaskHandle &handle);
        MODULES_EXPORT void onSuspend(std::pair<Threading::TaskQueue *, void *> data);

        MODULES_EXPORT void onEnter(const std::coroutine_handle<> &handle, Engine::Threading::TaskQueue *queue);
        MODULES_EXPORT void onReturn(const std::coroutine_handle<> &handle, Engine::Threading::TaskQueue *queue);

        struct MODULES_EXPORT TaskTracker {

            void onAssign(void *ident, std::source_location location);
            void onEnter(void *ident, std::chrono::high_resolution_clock::time_point timePoint = std::chrono::high_resolution_clock::now());
            void onReturn(void *ident, std::chrono::high_resolution_clock::time_point timePoint = std::chrono::high_resolution_clock::now());
            void onResume(void *ident, std::chrono::high_resolution_clock::time_point timePoint = std::chrono::high_resolution_clock::now());
            void onSuspend(void *ident, std::chrono::high_resolution_clock::time_point timePoint = std::chrono::high_resolution_clock::now());
            void onDestroy(void *ident);

            const std::source_location &getTraceback(void *ident);

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
            const std::map<void *, std::source_location> tasksInFlight() const;

            std::mutex mMutex;

            std::thread::id mThread;

            std::map<void *, std::source_location> mTasksInFlight;
        private:
            std::deque<Event> mEvents;
            
        };

    }

}
}