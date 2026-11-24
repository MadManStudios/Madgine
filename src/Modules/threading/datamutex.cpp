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

    DataMutex::DataMutex(std::string_view name, TaskQueue &taskQueue)
        : mName(name)
        , mTaskQueue(taskQueue)
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
        bool success = &mTaskQueue == TaskQueue::currentThreadQueue() && mMutex.try_lock();
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
        if (!head)
            mMutex.unlock_shared();

        return head;
    }

    DataMutex::LockState *DataMutex::unlockWrite()
    {
        LockState *head = mWriteQueue.pop();
        if (!head)
            mMutex.unlock();

        return head;
    }

    void DataMutex::run(LockState *state)
    {
        std::atomic<LockState *> initial = nullptr;
        std::atomic<LockState *> *nextPtr = &initial;
        while (true) {            
            if (!state) {
                if (!mReadQueue.empty()) {
                    if (mMutex.try_lock_shared()) {
                        state = unlockRead();
                    }
                }
            }
            if (!state) {
                if (!mWriteQueue.empty()) {
                    if (mMutex.try_lock()) {
                        state = unlockWrite();
                    }
                }
            }

            if (!state)
                break;

            if (state->mMode == AccessMode::WRITE && &mTaskQueue != TaskQueue::currentThreadQueue()) {
                mTaskQueue.queue([this, state]() {
                    run(state);
                });
                break;
            }

            if (!state->onLockAcquired())
                break;

            *nextPtr = state;
            nextPtr = &state->mNext;

            state = unlockImpl(state);
        }
        *nextPtr = nullptr;

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
        default:
            throw 0;
        }
    }

}
}
