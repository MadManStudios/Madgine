#pragma once

namespace Engine {

template <typename T>
struct CoW {

    CoW() = default;

    CoW(const T &p)
        : mConstPtr(&p)
    {
    }

    CoW(T &&v)
        : mPtr(std::make_unique<T>(std::move(v)))
    {
    }

    CoW(const T &&v)
        : mPtr(std::make_unique<T>(std::move(v)))
    {
    }

    CoW(const CoW<T> &other)
        : mPtr(other.mPtr ? std::make_unique<T>(*other.mPtr) : nullptr)
        , mConstPtr(other.mConstPtr)
    {
    }

    CoW(CoW<T> &&other) = default;

    CoW<T> &operator=(const CoW<T> &other)
    {
        if (!other.mPtr) {
            mConstPtr = other.mConstPtr;
            mPtr.reset();
        } else if (!mPtr) {
            mPtr = std::make_unique<T>(*other);
            mConstPtr = nullptr;
        } else {
            *mPtr = *other;
        }
        return *this;
    }

    CoW<T> &operator=(CoW<T> &&other)
    {
        std::swap(mPtr, other.mPtr);
        std::swap(mConstPtr, other.mConstPtr);
        return *this;
    }

    bool operator==(const CoW<T> &other) const
    {
        return **this == *other;
    }

    operator T &()
    {
        promote();
        return *mPtr;
    }

    operator const T &() const
    {
        return mPtr ? *mPtr : *mConstPtr;
    }

    T &operator*()
    {
        promote();
        return *mPtr;
    }

    const T &operator*() const
    {
        return mPtr ? *mPtr : *mConstPtr;
    }

    T *operator&()
    {
        promote();
        return mPtr.get();
    }

    T *operator->()
    {
        promote();
        return mPtr.get();
    }

    const T *operator->() const
    {
        return mPtr ? mPtr.get() : mConstPtr;
    }

private:
    void promote()
    {
        if (!mPtr) {
            mPtr = mConstPtr ? std::make_unique<T>(*mConstPtr) : std::make_unique<T>();
            mConstPtr = nullptr;
        }
    }

private:
    std::unique_ptr<T> mPtr;
    const T *mConstPtr = nullptr;
};

}