#include "../python3lib.h"

#include "python3lock.h"

#include "../python3env.h"

namespace Engine {
namespace Scripting {
    namespace Python3 {

        Python3Lock::Python3Lock(Log::Log *log, std::stop_token st)
        {
            Python3Environment::lock(log, std::move(st));
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
            std::tie(mLog, mStopToken) = Python3Environment::unlock();
        }

        Python3Unlock::~Python3Unlock()
        {
            Python3Environment::lock(mLog, std::move(mStopToken));
        }

        Log::Log *Python3Unlock::log()
        {
            return mLog;
        }

        std::stop_token Python3Unlock::st()
        {
            return std::move(mStopToken);
        }

    }
}
}