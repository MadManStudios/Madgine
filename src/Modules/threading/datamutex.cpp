#include "../moduleslib.h"

#include "datamutex.h"

#include "Generic/execution/execution.h"

#include "taskqueue.h"

namespace Engine {
namespace Threading {

    DataMutex::Lock::Lock(DataMutex &mutex, AccessMode mode)
        : LockState { mutex, mode }
    {
        if (!mutex.lockImpl(this)) {
            std::unique_lock lock { mCvMutex };
            mCv.wait(lock, [this]() { return mWasTriggered; });
        }
    }

    DataMutex::Lock::~Lock()
    {
        mMutex.run(mMutex.unlockImpl(this));
    }

    bool DataMutex::Lock::onLockAcquired()
    {
        {
            std::unique_lock lock { mCvMutex };
            mWasTriggered = true;
        }
        mCv.notify_one();

        return false;
    }

    void DataMutex::Lock::complete()
    {
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
            mReadQueue.push(state);
            if (mMutex.try_lock_shared()) {
                run(unlockRead());
            }
        }

        return success;
    }

    bool DataMutex::lockWrite(LockState *state)
    {
        bool success = mMutex.try_lock();
        if (!success) {
            mWriteQueue.push(state);
            if (mMutex.try_lock()) {
                run(unlockWrite());
            }
        }

        return success;
    }

    DataMutex::LockState *DataMutex::unlockImpl(LockState *state)
    {
        switch (state->mMode) {
        case AccessMode::READ:
            return unlockRead();
        case AccessMode::WRITE:
            return unlockWrite();
        default:
            throw 0;
        }
    }

    DataMutex::LockState *DataMutex::unlockRead()
    {
        LockState *head = mReadQueue.pop();
        if (head)
            return head;

        mMutex.unlock_shared();

        return nullptr;
    }

    DataMutex::LockState *DataMutex::unlockWrite()
    {
        LockState *head = mWriteQueue.pop();
        if (head)
            return head;

        mMutex.unlock();

        return nullptr;
    }

    void DataMutex::run(LockState *state)
    {
        std::atomic<LockState *> initial = nullptr;
        std::atomic<LockState *> *nextPtr = &initial;
        while (state) {
            if (!state->onLockAcquired())
                break;

            *nextPtr = state;
            nextPtr = &state->mNext;

            LockState *next = unlockImpl(state);

            if (!next) {
                if (!mReadQueue.empty()) {
                    if (mMutex.try_lock_shared()) {
                        next = mReadQueue.pop();
                        if (!next)
                            mMutex.unlock_shared();
                    }
                }
            }
            if (!next) {
                if (!mWriteQueue.empty()) {
                    if (mMutex.try_lock()) {
                        next = mWriteQueue.pop();
                        if (!next)
                            mMutex.unlock();
                    }
                }
            }

            state = next;
        }

        state = initial;
        while (state) {
            LockState *next = state->mNext;
            state->complete();
            state = next;
        }
    }

    bool DataMutex::cancel(LockState *state)
    {
        switch (state->mMode) {
        case AccessMode::READ:
            return mReadQueue.extract(state);
            break;
        case AccessMode::WRITE:
            return mWriteQueue.extract(state);
            break;
        }
    }

}
}
