#include "../python3lib.h"

#include "python3lock.h"

#include "../python3env.h"

namespace Engine {
namespace Behavior {
    namespace Python3 {

        Python3Lock::Python3Lock(BehaviorReceiver *rec)
        {
            Python3Environment::lock(rec);
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
            mReceiver = Python3Environment::unlock();
        }

        Python3Unlock::~Python3Unlock()
        {
            Python3Environment::lock(mReceiver);
        }

        BehaviorReceiver *Python3Unlock::receiver() const
        {
            return mReceiver;
        }

    }
}
}