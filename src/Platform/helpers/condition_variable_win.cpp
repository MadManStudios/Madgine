#include "../platformlib.h"

#include <Windows.h>

#include "condition_variable.h"

namespace Engine {
namespace Platform {

    struct ThreadData {

        ThreadData()
            : mTimer(CreateWaitableTimerExW(nullptr, nullptr, CREATE_WAITABLE_TIMER_HIGH_RESOLUTION, TIMER_ALL_ACCESS))
            , mEvent(CreateEventW(nullptr, FALSE, FALSE, nullptr))
        {
            if (!mTimer) // flag unsupported before Windows 10 1803: falls back to the coarse timer resolution
                mTimer = CreateWaitableTimerExW(nullptr, nullptr, 0, TIMER_ALL_ACCESS);

            assert(mTimer);
            assert(mEvent);
        }

        ~ThreadData()
        {
            if (mTimer)
                CloseHandle(mTimer);
            if (mEvent)
                CloseHandle(mEvent);
        }

        HANDLE mTimer;
        HANDLE mEvent;
    };

    static ThreadData &sThreadData()
    {
        static thread_local ThreadData data;
        return data;
    }

    struct condition_variable::Waiter {

        void resume()
        {
            SetEvent(mEvent);
        }

        void set_done()
        {
            std::terminate(); // cancelling is not allowed!
        }

        HANDLE mEvent;
        std::atomic<Waiter *> mNext = nullptr;
    };

    condition_variable::condition_variable() = default;
    condition_variable::~condition_variable() = default;

    void condition_variable::notify_one() noexcept
    {
        Waiter *waiter = mWaiters.pop();
        if (waiter) {
            waiter->resume();
        }
    }

    void condition_variable::notify_all() noexcept
    {
        Execution::ConnectionStack<Waiter> waiters = std::move(mWaiters);
        while (Waiter *waiter = waiters.pop()) {
            waiter->resume();
        }
    }

    /* void condition_variable::wait()
    {

    }*/

    std::cv_status condition_variable::wait_until(std::unique_lock<std::mutex> &lock, std::chrono::steady_clock::time_point abs_time)
    {
        ThreadData &data = sThreadData();

        const auto remaining = std::chrono::duration_cast<std::chrono::nanoseconds>(abs_time - std::chrono::steady_clock::now());
        if (remaining <= std::chrono::nanoseconds::zero()) {
            lock.unlock();
            lock.lock();
            return std::cv_status::timeout;
        }

        LARGE_INTEGER due;
        due.QuadPart = -std::max<LONGLONG>(1, (remaining.count() + 99) / 100);

        bool success = SetWaitableTimer(data.mTimer, &due, 0, nullptr, nullptr, FALSE);
        assert(success);

        Waiter waiter { data.mEvent };

        mWaiters.push(&waiter);

        const HANDLE handles[2] = { data.mEvent, data.mTimer };

        lock.unlock();

        bool notified = WaitForMultipleObjects(2, handles, FALSE, INFINITE) == WAIT_OBJECT_0;

        lock.lock();

        if (!notified) {
            bool extracted = mWaiters.extract(&waiter);
            if (!extracted) {
                WaitForSingleObject(data.mEvent, INFINITE);
            }
        }

        return notified ? std::cv_status::no_timeout : std::cv_status::timeout;
    }

}
}