#pragma once

namespace Engine {

template <typename T, size_t FreeBits = std::bit_width(alignof(T) - 1)>
struct BitPtr {

    static constexpr size_t FreeBitCount = FreeBits;
    static constexpr uintptr_t Mask = (1 << FreeBitCount) - 1;

    constexpr BitPtr() = default;

    constexpr BitPtr(T *p)
    {
        setMasked(p);
    }

    BitPtr &operator=(T *p)
    {
        setMasked(p);
        return *this;
    }

    T *operator->() const
    {
        return maskedPtr();
    }

    T &operator*() const
    {
        return *maskedPtr();
    }

    constexpr operator T *() const
    {
        return maskedPtr();
    }

    bool operator==(nullptr_t) const
    {
        return maskedPtr() == nullptr;
    }

    auto operator<=>(T *other)
    {
        return maskedPtr() <=> other;
    }

    explicit constexpr operator bool() const
    {
        return maskedPtr() != nullptr;
    }

protected:
    T *maskedPtr() const
    {
        return reinterpret_cast<T *>(reinterpret_cast<uintptr_t>(mPtr) & ~Mask);
    }

    void setMasked(T *p)
    {
        uintptr_t &ref = reinterpret_cast<uintptr_t &>(mPtr);
        uintptr_t masked = ref & Mask;
        ref = (reinterpret_cast<uintptr_t>(p) & ~Mask) | masked;
    }

private:
    T *mPtr;
};

template <typename T>
struct BitUniquePtr : private BitPtr<T> {
    using BitPtr<T>::FreeBitCount;

    constexpr BitUniquePtr()
        : BitPtr<T>(nullptr)
    {
    }

    explicit BitUniquePtr(T *p)
        : BitPtr<T>(p)
    {
    }

    BitUniquePtr(std::unique_ptr<T> p)
        : BitPtr<T>(p.release())
    {
    }

    BitUniquePtr(BitUniquePtr &&other)
        : BitPtr<T>(other.release())
    {
    }

    ~BitUniquePtr()
    {
        reset();
    }

    void reset()
    {
        T *p = this->maskedPtr();
        if (p) {
            delete p;
            this->setMasked(nullptr);
        }
    }

    T *release()
    {
        T *p = this->maskedPtr();
        if (p) {
            this->setMasked(nullptr);
        }
        return p;
    }

    using BitPtr<T>::operator->;
    using BitPtr<T>::operator*;
};

}