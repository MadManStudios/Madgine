#include "../python3lib.h"

#include "python3lock.h"

#include "../python3env.h"

namespace Engine {
namespace Behavior {
    namespace Python3 {

        Python3Lock::Python3Lock(BehaviorReceiver *rec, Platform::Log::Log *log)
        {
            Python3Environment::lock(rec, log);
        }

        Python3Lock::Python3Lock(Platform::Log::Log *log)
        {
            Python3Environment::lock(nullptr, log);
        }

        Python3Lock::~Python3Lock()
        {
            Python3Environment::unlock();
        }

        Python3InnerLock::Python3InnerLock()
            : mState(Python3Environment::lock())
        {
        }

        Python3InnerLock::Python3InnerLock(Python3InnerLock &&other)
            : mState(std::exchange(other.mState, std::nullopt))
        {
        }

        Python3InnerLock::~Python3InnerLock()
        {
            if (mState)
                Python3Environment::unlock(*mState);
        }

        Python3Unlock::Python3Unlock()
        {
            std::tie(mReceiver, mLog) = Python3Environment::unlock();
        }

        Python3Unlock::~Python3Unlock()
        {
            Python3Environment::lock(mReceiver, mLog);
        }

        BehaviorReceiver *Python3Unlock::fetchReceiver() 
        {
            if (mLog == Platform::Log::get_log(mReceiver))
                mLog = nullptr;
            return std::exchange(mReceiver, nullptr);
        }

        Platform::Log::Log *Python3Unlock::log() const
        {
            return mLog;
        }

    }
}
}