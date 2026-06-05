#pragma once

namespace Engine {
namespace Behavior {
    namespace Python3 {

        struct MADGINE_PYTHON3_EXPORT Python3Lock {
            Python3Lock(BehaviorReceiver *rec = nullptr, Platform::Log::Log *log = nullptr);
            Python3Lock(Platform::Log::Log *log);
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

            BehaviorReceiver *fetchReceiver();
            Platform::Log::Log *log() const;

        private:
            BehaviorReceiver *mReceiver;
            Platform::Log::Log *mLog;
        };
    }
}
}