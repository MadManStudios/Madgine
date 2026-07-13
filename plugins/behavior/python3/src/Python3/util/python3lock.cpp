#include "../python3lib.h"

#include "python3lock.h"

#include "../python3streamredirect.h"
#include "pyexecution.h"

namespace Engine {
namespace Behavior {
    namespace Python3 {

        Python3Lock::Python3Lock(BehaviorReceiver *rec, Platform::Log::Log *log)
        {
            [[maybe_unused]] bool locked = lock(rec, log);
            assert(locked);
        }

        Python3Lock::Python3Lock(Platform::Log::Log *log)
        {
            [[maybe_unused]] bool locked = lock(nullptr, log);
            assert(locked);
        }

        Python3Lock::~Python3Lock()
        {
            unlock();
        }

        Python3InnerLock::Python3InnerLock()
            : mLocked(PyGILState_Ensure() == PyGILState_UNLOCKED)
        {
            LOG_DEBUG("[" << std::this_thread::get_id() << "] Inner Lock");
        }

        Python3InnerLock::Python3InnerLock(Python3InnerLock &&other)
            : mLocked(std::exchange(other.mLocked, false))
        {
        }

        Python3InnerLock::~Python3InnerLock()
        {
            if (mLocked) {
                LOG_DEBUG("[" << std::this_thread::get_id() << "] Inner Release");
                PyGILState_Release(PyGILState_UNLOCKED);
            }
        }

        Python3Suspend::Python3Suspend()
        {
            auto [receiver, log] = std::exchange(executionState(), {});

            mReceiver = receiver;
            mLog = log;

            LOG_DEBUG("[" << std::this_thread::get_id() << ", " << PyThreadState_Get() << "] Suspend: " << mReceiver);

            mThreadSave = PyEval_SaveThread();
        }

        Python3Suspend::~Python3Suspend()
        {
            PyEval_RestoreThread(mThreadSave);

            LOG_DEBUG("[" << std::this_thread::get_id() << ", " << PyThreadState_Get() << "] Resume: " << mReceiver);

            auto [receiver, log] = std::exchange(executionState(), { mReceiver, mLog });

            assert(receiver == nullptr);
            assert(log == nullptr);
        }

        BehaviorReceiver *Python3Suspend::fetchReceiver()
        {
            if (mLog == Platform::Log::get_log(mReceiver))
                mLog = nullptr;
            return std::exchange(mReceiver, nullptr);
        }

        BehaviorReceiver *Python3Suspend::receiver()
        {
            return mReceiver;
        }

        Platform::Log::Log *Python3Suspend::log() const
        {
            return mLog;
        }

    }
}
}