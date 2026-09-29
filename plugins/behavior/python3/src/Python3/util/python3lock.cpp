#include "../python3lib.h"

#include "python3lock.h"

#include "../python3streamredirect.h"
#include "pyexecution.h"

namespace Engine {
namespace Behavior {
    namespace Python3 {

        Python3Lock::Python3Lock()
        {
            [[maybe_unused]] bool locked = lock();
            assert(locked);
        }

        Python3Lock::~Python3Lock()
        {
            unlock();
        }

        Python3InnerLock::Python3InnerLock()
            : mLocked(PyGILState_Ensure() == PyGILState_UNLOCKED)
        {
        }

        Python3InnerLock::Python3InnerLock(Python3InnerLock &&other)
            : mLocked(std::exchange(other.mLocked, false))
        {
        }

        Python3InnerLock::~Python3InnerLock()
        {
            if (mLocked) {
                PyGILState_Release(PyGILState_UNLOCKED);
            }
        }

        Python3Suspend::Python3Suspend()
            : mThreadSave(PyEval_SaveThread())
        {
        }

        Python3Suspend::~Python3Suspend()
        {
            PyEval_RestoreThread(mThreadSave);
        }

    }
}
}