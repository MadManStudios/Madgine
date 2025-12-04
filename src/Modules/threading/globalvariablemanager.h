#pragma once

#include "Generic/any.h"

namespace Engine {
namespace Threading {

    struct GlobalVariableManager {

        GlobalVariableManager(std::function<Any()> ctor)
            : mCtor(std::move(ctor))
            , mMutex(std::make_unique<std::shared_mutex>())
        {
        }

        void reset()
        {
            std::unique_lock lock(*mMutex);
            mCtor = {};
            mData.clear();
        }

        Any &operator[](size_t index)
        {
            std::shared_lock lock(*mMutex);
            if (mData.size() <= index || !mData[index]) {
                mMutex->unlock_shared();
                mMutex->lock();
                if (mData.size() <= index)
                    mData.resize(index + 1);
                if (!mData[index])
                    mData[index] = mCtor();
                mMutex->unlock();
                mMutex->lock_shared();
            }
            return mData[index];
        }

        void remove(size_t index)
        {
            bool available;
            {
                std::shared_lock lock(*mMutex);
                available = (mData.size() > index);
            }
            if (available) {
                std::unique_lock lock2(*mMutex);
                mData[index] = {};
            }
        }

        std::vector<Any> mData;

        std::function<Any()> mCtor;

        std::unique_ptr<std::shared_mutex> mMutex;
    };

}
}