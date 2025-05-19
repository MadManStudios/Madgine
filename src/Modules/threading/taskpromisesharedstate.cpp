#include "../moduleslib.h"

#include "taskpromisesharedstate.h"

#include "taskhandle.h"

namespace Engine {
namespace Threading {

    TaskPromiseSharedStateBase::TaskPromiseSharedStateBase(bool ready)
        : mAttached(ready)
        , mDestroyed(ready)
        , mDone(ready)
    {
    }

    TaskPromiseSharedStateBase::~TaskPromiseSharedStateBase()
    {
    }

    void TaskPromiseSharedStateBase::attach()
    {
        std::lock_guard guard { mMutex };
        assert(!mAttached);
        mAttached = true;
    }

    void TaskPromiseSharedStateBase::finalize()
    {
        std::lock_guard guard { mMutex };
        mDone = true;
    }

    void TaskPromiseSharedStateBase::notifyDestroyed()
    {
        std::lock_guard guard { mMutex };
        mDestroyed = true;
    }

}
}