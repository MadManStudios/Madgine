#pragma once

namespace Engine {

struct UniqueOpaquePtr {

    UniqueOpaquePtr() = default;
    UniqueOpaquePtr(const UniqueOpaquePtr &) = delete;
    UniqueOpaquePtr(UniqueOpaquePtr &&other)
        : mPtr(std::exchange(other.mPtr, 0))
    {
    }
    ~UniqueOpaquePtr()
    {
        if (mPtr)
            std::terminate();
    }

    UniqueOpaquePtr &operator=(const UniqueOpaquePtr &) = delete;
    UniqueOpaquePtr &operator=(UniqueOpaquePtr &&other)
    {
        std::swap(mPtr, other.mPtr);
        return *this;
    }

    constexpr explicit operator bool() const
    {
        return mPtr;
    }

    uintptr_t get() const
    {
        return mPtr;
    }

    template <typename T>
    T &setupAs()
    {
        assert(!mPtr);
        return reinterpret_cast<T &>(mPtr);
    }

    template <typename T>
    const T &as() const
    {
        assert(mPtr);
        return reinterpret_cast<const T &>(mPtr);
    }

    template <typename T>
    T &as()
    {
        assert(mPtr);
        return reinterpret_cast<T &>(mPtr);
    }

    template <typename T>
    T release() noexcept
    {
        assert(mPtr);
        T result = std::move(reinterpret_cast<T &>(mPtr));
        mPtr = 0;
        return result;
    }

private:
    uintptr_t mPtr = 0;
};

}