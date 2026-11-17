#pragma once

#if WINDOWS
#    include "Generic/execution/container/stack.h"
#endif

namespace Engine {
namespace Platform {

#if !WINDOWS
    using condition_variable = std::condition_variable;
#else

    struct PLATFORM_EXPORT condition_variable {

        condition_variable();
        ~condition_variable();

        void notify_one() noexcept;
        void notify_all() noexcept;
        //void wait();

        std::cv_status wait_until(std::unique_lock<std::mutex> &lock, std::chrono::steady_clock::time_point abs_time);

    private:
        struct Waiter;
        Execution::ConnectionStack<Waiter> mWaiters;
    };

#endif

}
}