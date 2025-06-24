#pragma once

#include "container/stack.h"
#include "stop_callback.h"

namespace Engine {
namespace Execution {

    struct StopSource {

        StopSource() = default;
        StopSource(StopSource &&) = delete;

        bool registerCallback(StopCallback *callback)
        {
            std::unique_lock lock { mStack.mutex() };
            if (mStopped) {
                return false;
            } else {
                mStack.push(callback, lock);
                return true;
            }
        }
        bool unregisterCallback(StopCallback *callback)
        {
            return mStack.extract(callback);
        }

        bool stop_requested() const
        {
            std::lock_guard guard { mStack.mutex() };
            return static_cast<bool>(mStopped);
        }

        StopToken get_token()
        {
            return this;
        }

        bool request_stop()
        {
            std::unique_lock lock { mStack.mutex() };
            if (mStopped)
                return false;
            mStopped = true;
            lock.unlock();
            ConnectionStack<StopCallback> stack = std::move(mStack);

            while (StopCallback *callback = stack.pop()) {
                callback->stopRequested();
            }
            return true;
        }

    protected:
        ConnectionStack<StopCallback> mStack;
        bool mStopped = false;
    };

}
}