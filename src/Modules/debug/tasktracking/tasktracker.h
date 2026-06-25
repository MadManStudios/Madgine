#pragma once

#include "Platform/debug/stacktrace.h"

namespace Engine {
namespace Debug {

    namespace Tasks {

        MODULES_EXPORT void onAssign(const std::coroutine_handle<> &handle, Threading::TaskQueue *queue, std::source_location location);
        MODULES_EXPORT void onDestroy(Threading::TaskPromiseBase &promise);

        MODULES_EXPORT void onResume(Threading::TaskQueue *queue, void *id, uint16_t depth);
        MODULES_EXPORT uint16_t onSuspend(Threading::TaskQueue *queue, void *id);

        MODULES_EXPORT void onEnter(const std::coroutine_handle<> &handle, Threading::TaskQueue *queue);
        MODULES_EXPORT void onReturn(void *id, Threading::TaskQueue *queue);

        struct MODULES_EXPORT TaskTracker {

            void onAssign(void *ident, std::source_location location);
            void onEnter(void *ident, std::chrono::high_resolution_clock::time_point timePoint = std::chrono::high_resolution_clock::now());
            void onReturn(void *ident, std::chrono::high_resolution_clock::time_point timePoint = std::chrono::high_resolution_clock::now());
            void onResume(void *ident, uint16_t depth, std::chrono::high_resolution_clock::time_point timePoint = std::chrono::high_resolution_clock::now());
            uint16_t onSuspend(void *ident, std::chrono::high_resolution_clock::time_point timePoint = std::chrono::high_resolution_clock::now());
            void onDestroy(void *ident);

            const std::source_location &getTraceback(void *ident);

            struct Event {
                enum Type : uint8_t {
                    ENTER,
                    RETURN,
                    RESUME,
                    SUSPEND
                } mType;
                uint16_t mDepth;
                void *mIdentifier;
                std::chrono::high_resolution_clock::time_point mTimePoint = std::chrono::high_resolution_clock::now();

                Event(Type type, void *ident = nullptr, std::chrono::high_resolution_clock::time_point timePoint = std::chrono::high_resolution_clock::now(), uint16_t depth = 0)
                    : mType(type)
                    , mIdentifier(ident)
                    , mTimePoint(timePoint)
                    , mDepth(depth)
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
            uint16_t mDepth = 0;
            std::stack<uint16_t> mDepthStack;
            
        };

    }

}
}