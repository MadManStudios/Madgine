#pragma once

namespace Engine {

template <typename T = void>
struct CoroutineHandle {

    CoroutineHandle() noexcept = default;
    CoroutineHandle(const CoroutineHandle &) = delete;
    CoroutineHandle(CoroutineHandle &&other) noexcept
        : mHandle(std::exchange(other.mHandle, nullptr))
    {
    }
    CoroutineHandle(std::coroutine_handle<T> handle) noexcept
        : mHandle(handle)
    {
    }
    template <std::derived_from<T> U>
    CoroutineHandle(std::coroutine_handle<U> handle) noexcept
        : CoroutineHandle(CoroutineHandle<U> { handle })
    {
    }
    template <std::derived_from<T> U>
    CoroutineHandle(CoroutineHandle<U> &&handle) noexcept
        : CoroutineHandle(fromPromise(handle.release().promise()))
    {
    }
    ~CoroutineHandle() noexcept
    {
        reset();
    }

    CoroutineHandle &operator=(const CoroutineHandle &) = delete;
    CoroutineHandle &operator=(CoroutineHandle &&other) noexcept
    {
        std::swap(mHandle, other.mHandle);
        return *this;
    }

    std::coroutine_handle<T> release() noexcept
    {
        return std::exchange(mHandle, nullptr);
    }

    void reset()
    {
        if (mHandle) {
            mHandle.destroy();
            mHandle = nullptr;
        }
    }

    void resume()
    {
        mHandle.resume();
    }

    bool done()
    {
        return mHandle.done();
    }

    std::coroutine_handle<T> get()
    {
        return mHandle;
    }

    T &promise() const
    {
        return mHandle.promise();
    }

    T *operator->()
    {
        return &promise();
    }

    T &operator*() const
    {
        return promise();
    }

    explicit constexpr operator bool() const noexcept
    {
        return static_cast<bool>(mHandle);
    }

    static CoroutineHandle fromPromise(T &address)
    {
        return { std::coroutine_handle<T>::from_promise(address) };
    }

    template <std::derived_from<T> U>
    CoroutineHandle<U> cast()&&
    {
        CoroutineHandle<U> result = CoroutineHandle<U>::fromPromise(static_cast<U &>(mHandle.promise()));
        mHandle = {};
        return result;
    }

private:
    std::coroutine_handle<T> mHandle;
};

template <>
struct CoroutineHandle<void> {

    CoroutineHandle() = default;
    CoroutineHandle(const CoroutineHandle &) = delete;
    CoroutineHandle(CoroutineHandle &&other)
        : mHandle(std::exchange(other.mHandle, nullptr))
    {
    }
    CoroutineHandle(std::coroutine_handle<> handle)
        : mHandle(handle)
    {
    }
    template <typename U>
    CoroutineHandle(std::coroutine_handle<U> handle)
        : CoroutineHandle(CoroutineHandle<U> { handle })
    {
    }
    template <typename U>
    CoroutineHandle(CoroutineHandle<U> &&handle)
        : mHandle(handle.release())
    {
    }
    ~CoroutineHandle()
    {
        reset();
    }

    CoroutineHandle &operator=(const CoroutineHandle &) = delete;
    CoroutineHandle &operator=(CoroutineHandle &&other)
    {
        std::swap(mHandle, other.mHandle);
        return *this;
    }

    std::coroutine_handle<> release()
    {
        return std::exchange(mHandle, nullptr);
    }

    void reset()
    {
        if (mHandle) {
            mHandle.destroy();
            mHandle = nullptr;
        }
    }

    void resume()
    {
        mHandle.resume();
    }

    bool done()
    {
        return mHandle.done();
    }

    std::coroutine_handle<> get()
    {
        return mHandle;
    }

    explicit operator bool() const
    {
        return static_cast<bool>(mHandle);
    }

private:
    std::coroutine_handle<> mHandle;
};
}