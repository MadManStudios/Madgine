#include "../../moduleslib.h"

#include "profilerthread.h"

#include "profiler.h"

#include "../../threading/workgroup.h"


#if ENABLE_THREADING
#    include "Interfaces/threading/threadapi.h"
#endif

namespace Engine {

namespace Debug {

    namespace Profiler {

        ProfilerThread::ProfilerThread()
            : mStats("<unnamed-thread>")
        {
            updateName();
            mCurrent = &mStats;
            mStats.start();
        }

        ProfilerThread::ProfilerThread(const char *name)
            : mStats(name)
        {
            mCurrent = &mStats;
            mStats.start();
        }

        ProfilerThread::~ProfilerThread()
        {
            mStats.stop();

#if ENABLE_THREADING
            if (Threading::WorkGroup::isInitialized())
                Profiler::getCurrent().unregisterThread(this);
#endif
        }

        void ProfilerThread::updateName()
        {
#if ENABLE_THREADING
            mId = Threading::getCurrentThreadName();
#else
            mId = "Main";
#endif
            mStats.setFunctionName(mId.c_str());
        }

    }
}
}
