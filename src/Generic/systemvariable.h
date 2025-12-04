#pragma once

namespace Engine {

template <typename T, T initial = T {}>
struct SystemVariable {
public:
    SystemVariable()
        : mValue(initial)
    {
    }

    SystemVariable(T t)
        : mValue(t)
    {
    }

    void wait()
    {
#if __cpp_lib_atomic_wait <= 201907L
        if (mValue == initial) {
            std::unique_lock lock(mMutex);
            mCond.wait(lock, [this]() { return mValue != initial; });
        }
#else
        mValue.wait(initial);
#endif
    }

    operator T() const
    {
        return mValue;
    }

    SystemVariable<T, initial> &operator=(T t)
    {
        mValue = t;
#if __cpp_lib_atomic_wait <= 201907L
        mCond.notify_all();
#endif
        return *this;
    }

private:
    std::atomic<T> mValue;
#if __cpp_lib_atomic_wait <= 201907L
    std::mutex mMutex;
    std::condition_variable mCond;
#endif
};

}