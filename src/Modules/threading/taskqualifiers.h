#pragma once

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

}
}