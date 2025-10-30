#pragma once

namespace Engine {
namespace Scripting {
    namespace Python3 {

        struct MADGINE_PYTHON3_EXPORT Python3Lock {
            Python3Lock(Log::Log *log = nullptr, Execution::StopToken st = {});
            Python3Lock(const Python3Lock &) = delete;
            ~Python3Lock();

        };

        struct MADGINE_PYTHON3_EXPORT Python3InnerLock {
            Python3InnerLock();
            Python3InnerLock(const Python3InnerLock &) = delete;
            Python3InnerLock(Python3InnerLock &&);
            ~Python3InnerLock();

        private:
            std::optional<PyGILState_STATE> mState;
        };

        struct MADGINE_PYTHON3_EXPORT Python3Unlock {
            Python3Unlock();
            Python3Unlock(const Python3Unlock &) = delete;
            ~Python3Unlock();

            Log::Log *log();
            Execution::StopToken st();

        private:
            Log::Log *mLog;
            Execution::StopToken mStopToken;
        };
    }
}
}