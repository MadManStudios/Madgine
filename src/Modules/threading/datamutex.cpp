#include "../moduleslib.h"

#include "datamutex.h"

#include "Generic/execution/execution.h"

#include "taskqueue.h"

namespace Engine {
namespace Threading {

    DataMutex::Lock::Lock(DataMutex &mutex, AccessMode mode)
        : LockState { &mutex, mode }
    {
        if (!mutex.lockImpl(this)) {
            std::unique_lock lock { mCvMutex };
            mCv.wait(lock, [this]() { return mWasTriggered; });
        }
    }

    DataMutex::Lock::~Lock()
    {
        if (mMutex)
            mMutex->unlockImpl(this);
    }

    bool DataMutex::Lock::isHeld() const
    {
        return mMutex;
    }

    void DataMutex::Lock::onLockAcquired()
    {
        {
            std::unique_lock lock { mCvMutex };
            mWasTriggered = true;
        }
        mCv.notify_one();
    }

    DataMutex::DataMutex(std::string_view name)
        : mName(name)
    {
    }

    DataMutex::~DataMutex()
    {
        assert(mReadQueue.empty());
        assert(mWriteQueue.empty());
    }

    const std::string &DataMutex::name() const
    {
        return mName;
    }

    DataMutex::Lock DataMutex::lock(AccessMode mode)
    {
        return { *this, mode };
    }

    bool DataMutex::lockImpl(LockState *state)
    {
        switch (state->mMode) {
        case AccessMode::READ:
            return lockRead(state);
        case AccessMode::WRITE:
            return lockWrite(state);
        default:
            throw 0;
        }
    }

    bool DataMutex::lockRead(LockState *state)
    {

        bool success = mMutex.try_lock_shared();
        if (!success) {
            bool wasFirstItem = mReadQueue.push(state);
            if (wasFirstItem) {
                if (mMutex.try_lock_shared()) {
                    if (!unrollRead())
                        mMutex.unlock_shared();
                }
            }
        }

        return success;
    }

    bool DataMutex::lockWrite(LockState *state)
    {
        bool success = mMutex.try_lock();
        if (!success) {
            bool wasFirstItem = mWriteQueue.push(state);
            if (wasFirstItem) {
                if (mMutex.try_lock()) {
                    if (!unrollWrite())
                        mMutex.unlock();
                }
            }
        }

        return success;
    }

    void DataMutex::unlockImpl(LockState *state)
    {
        switch (state->mMode) {
        case AccessMode::READ:
            return unlockRead(state);
        case AccessMode::WRITE:
            return unlockWrite(state);
        default:
            throw 0;
        }
    }

    void DataMutex::unlockRead(LockState *state)
    {
        if (!unrollRead()) {
            mMutex.unlock_shared();
            if (!mWriteQueue.empty()) {
                if (mMutex.try_lock()) {
                    if (!unrollWrite())
                        mMutex.unlock();
                }
            }
        }
    }

    void DataMutex::unlockWrite(LockState *state)
    {
        if (!unrollWrite()) {
            mMutex.unlock();
            if (!mReadQueue.empty()) {
                if (mMutex.try_lock_shared()) {
                    if (!unrollRead())
                        mMutex.unlock_shared();
                }
            }
        }
    }

    bool DataMutex::unrollRead()
    {
        LockState *head = mReadQueue.pop();
        if (head) {
            assert(head->mMode == AccessMode::READ);
            head->onLockAcquired();
            return true;
        } else {
            return false;
        }
    }

    bool DataMutex::unrollWrite()
    {
        LockState *head = mWriteQueue.pop();
        if (head) {
            assert(head->mMode == AccessMode::WRITE);
            head->onLockAcquired();
            return true;
        } else {
            return false;
        }
    }
}
}
