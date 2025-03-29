#pragma once

namespace Engine {

template <typename T, bool simplifyPointer = true>
struct Proxy {

    template <typename... Args>
    Proxy(Args&&... args)
        : mData(std::forward<Args>(args)...)
    {
    }

    T *operator->() { return &mData; }

    T &operator*() { return mData; }

    T *operator&() { return &mData; }

    operator T &() { return mData; }

    Proxy<T, simplifyPointer> &operator=(const T &t)
    {
        mData = t;
        return *this;
    }

    Proxy<T, simplifyPointer> &operator=(T &&t)
    {
        mData = std::forward<T>(t);
        return *this;
    }

private:
    T mData;
};

template <typename T>
struct Proxy<T *, true> {

    Proxy(T *t = nullptr)
        : mPtr(t)
    {
    }

    T *operator->() { return mPtr; }

    T &operator*() { return *mPtr; }

    T **operator&() { return &mPtr; }

    operator T *&() { return mPtr; }

    Proxy<T *, true> &operator=(T *t)
    {
        mPtr = t;
        return *this;
    }

private:
    T *mPtr;
};

}