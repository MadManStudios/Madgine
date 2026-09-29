#pragma once

namespace Engine {
namespace Behavior {
    namespace Python3 {

        struct MADGINE_PYTHON3_EXPORT Python3Lock {
            Python3Lock();            
            Python3Lock(const Python3Lock &) = delete;
            ~Python3Lock();
        };

        struct MADGINE_PYTHON3_EXPORT Python3InnerLock {
            Python3InnerLock();
            Python3InnerLock(const Python3InnerLock &) = delete;
            Python3InnerLock(Python3InnerLock &&);
            ~Python3InnerLock();

        private:
            bool mLocked;
        };

        struct MADGINE_PYTHON3_EXPORT Python3Suspend {
            Python3Suspend();
            Python3Suspend(const Python3Suspend &) = delete;
            ~Python3Suspend();

        private:
            PyThreadState *mThreadSave;
        };
    }
}
}