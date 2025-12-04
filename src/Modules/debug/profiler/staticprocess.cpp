#include "../../moduleslib.h"

#include "staticprocess.h"

#include "../../threading/threadlocal.h"
#include "profilerthread.h"

namespace Engine {

namespace Debug {

    namespace Profiler {

        extern THREADLOCAL(ProfilerThread) sThread;

        void StaticProcess::start()
        {
            if (mStats.start()) {
                mPrevious = sThread->mCurrent;
                sThread->mCurrent = &mStats;
            }
        }

        void StaticProcess::stop()
        {
            std::optional<ProcessStats::Data> d = mStats.stop();
            if (d) {
                assert(sThread->mCurrent == &mStats);
                sThread->mCurrent = mPrevious;
                if (mPrevious) {
                    mPrevious = nullptr;
                    if (!mParent) {
                        mParent = sThread->mCurrent->updateChild(&mStats, *d);
                    } else {
                        mParent->second += *d;
                    }
                }
            }
        }

    }
}
}
