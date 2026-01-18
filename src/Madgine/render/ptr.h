#pragma once

namespace Engine {
namespace Render {

    template <typename T>
    struct GPUPtr {

        GPUPtr() = default;

        template <typename U>
            requires(std::convertible_to<U *, T *> && !std::is_array_v<U>)
        GPUPtr(GPUPtr<U> other)
            : mPtr(other.mPtr)
        {
        }

        template <typename U>
        explicit GPUPtr(GPUPtr<U> other)
            : mPtr(std::static_pointer_cast<T>(other.mPtr))
        {
        }

        template <typename Deleter>
        GPUPtr(T *ptr, Deleter &&deleter)
            : mPtr(ptr, std::forward<Deleter>(deleter))
        {
        }

        explicit operator bool() const
        {
            return static_cast<bool>(mPtr);
        }

        void *get() const
        {
            return mPtr.get();
        }

        size_t size() const
        {
            return sizeof(T);
        }

    private:
        template <typename U>
        friend struct GPUPtr;

        std::shared_ptr<T> mPtr;
    };

    template <>
    struct GPUPtr<void> {

        GPUPtr() = default;

        template <typename U>
            requires(!std::is_array_v<U>)
        GPUPtr(GPUPtr<U> other)
            : mPtr(other.mPtr)
            , mSize(sizeof(U))
        {
        }

        template <typename Deleter>
        GPUPtr(void *ptr, size_t size, Deleter &&deleter)
            : mPtr(ptr, std::forward<Deleter>(deleter))
            , mSize(size)
        {
        }

        explicit operator bool() const
        {
            return static_cast<bool>(mPtr);
        }

        void *get() const
        {
            return mPtr.get();
        }

        size_t size() const
        {
            return mSize;
        }

    private:
        template <typename U>
        friend struct GPUPtr;

        std::shared_ptr<void> mPtr;

        size_t mSize;
    };

    template <typename T>
    struct GPUPtr<T[]> {

        GPUPtr() = default;

        template <typename U>
            requires std::convertible_to<U *, T *>
        GPUPtr(GPUPtr<U[]> other)
            : mPtr(other.mPtr)
            , mCount(other.mCount)
        {
        }

        template <typename U>
        explicit GPUPtr(GPUPtr<U[]> other)
            : mPtr(std::static_pointer_cast<T[]>(other.mPtr))
            , mCount(other.mCount)
        {
        }

        template <typename Deleter>
        GPUPtr(T (&ptr)[], size_t count, Deleter &&deleter)
            : mPtr(ptr, std::forward<Deleter>(deleter))
            , mCount(count)
        {
        }

        explicit operator bool() const
        {
            return static_cast<bool>(mPtr);
        }

        T *get() const
        {
            return mPtr.get();
        }

        size_t size() const
        {
            return sizeof(T) * mCount;
        }

        size_t elementSize() const
        {
            return sizeof(T);
        }

        size_t elementCount() const
        {
            return mCount;
        }

    private:
        template <typename U>
        friend struct GPUPtr;

        std::shared_ptr<T[]> mPtr;

        size_t mCount;
    };

    template <>
    struct GPUPtr<Void[]> {

        GPUPtr() = default;

        template <typename U>
        GPUPtr(GPUPtr<U[]> other)
            : mPtr(other.mPtr)
            , mElementSize(sizeof(U))
            , mCount(other.mCount)
        {
        }

        template <typename Deleter>
        GPUPtr(void *ptr, size_t elementSize, size_t count, Deleter &&deleter)
            : mPtr(ptr, std::forward<Deleter>(deleter))
            , mElementSize(elementSize)
            , mCount(count)
        {
        }

        explicit operator bool() const
        {
            return static_cast<bool>(mPtr);
        }

        void *get() const
        {
            return mPtr.get();
        }

        size_t size() const
        {
            return mElementSize * mCount;
        }
                
        size_t elementSize() const
        {
            return mElementSize;
        }

        size_t elementCount() const
        {
            return mCount;
        }

    private:
        template <typename U>
        friend struct GPUPtr;

        std::shared_ptr<void> mPtr;

        size_t mElementSize;
        size_t mCount;
    };

    template <typename T>
    using GPUArrayPtr = GPUPtr<T[]>;

}
}