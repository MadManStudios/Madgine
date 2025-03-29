#pragma once

#include "processstats.h"

namespace Engine {
namespace Debug {
    namespace Profiler {

        struct MODULES_EXPORT ProfilerThread {
            ProfilerThread();
            ProfilerThread(const char *name);
            ProfilerThread(const ProfilerThread &) = delete;
            ProfilerThread(ProfilerThread &&) = delete;
            ~ProfilerThread();

            void updateName();

            std::string mId;

            ProcessStats mStats;
            ProcessStats *mCurrent;
        };

    }
}
}