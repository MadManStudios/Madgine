#pragma once


namespace Engine {

struct OffsetPtr {

    constexpr OffsetPtr() = default;

    explicit constexpr OffsetPtr(uintptr_t offset)
        : mOffset(offset)
    {
    }

    template <typename T, typename Base>
    explicit OffsetPtr(type_holder_t<T>, type_holder_t<Base>)
        : mOffset(reinterpret_cast<uintptr_t>(static_cast<Base *>(static_cast<T *>(reinterpret_cast<void *>(0x1)))) - 1)
    {
    }

    explicit OffsetPtr(const void *from, const void *to)
        : mOffset(static_cast<const std::byte *>(to) - static_cast<const std::byte *>(from))
    {
    }

    constexpr uintptr_t offset() const
    {
        return mOffset;
    }

    constexpr void reset()
    {
        mOffset = std::numeric_limits<uintptr_t>::max();
    }

    constexpr std::strong_ordering operator<=>(const OffsetPtr &other) const = default;
    constexpr std::strong_ordering operator<=>(uintptr_t offset) const
    {
        return mOffset <=> offset;
    }

    constexpr explicit operator bool() const
    {
        return mOffset != std::numeric_limits<uintptr_t>::max();
    }

    constexpr OffsetPtr &operator+=(uintptr_t offset)
    {
        mOffset += offset;
        return *this;
    }

    friend constexpr const std::byte *operator+(const std::byte *p, OffsetPtr offset)
    {
        return p + offset.mOffset;
    }

    friend constexpr const std::byte *operator-(const std::byte *p, OffsetPtr offset)
    {
        return p - offset.mOffset;
    }

    friend constexpr std::byte *operator+(std::byte *p, OffsetPtr offset)
    {
        return p + offset.mOffset;
    }

    friend constexpr std::byte *operator-(std::byte *p, OffsetPtr offset)
    {
        return p - offset.mOffset;
    }

    constexpr OffsetPtr operator+(uintptr_t offset) const
    {
        return OffsetPtr { mOffset + offset };
    }

    friend constexpr OffsetPtr operator+(uintptr_t offset, OffsetPtr ptr)
    {
        return ptr + offset;
    }

    constexpr OffsetPtr operator-(uintptr_t offset) const
    {
        return OffsetPtr { mOffset - offset };
    }

    friend constexpr OffsetPtr operator-(uintptr_t offset, OffsetPtr ptr)
    {
        return ptr - offset;
    }

    constexpr OffsetPtr operator*(uintptr_t factor) const
    {
        return OffsetPtr { mOffset * factor };
    }

    friend constexpr OffsetPtr operator*(uintptr_t factor, OffsetPtr ptr)
    {
        return ptr * factor;
    }

    constexpr OffsetPtr operator/(uintptr_t factor) const
    {
        assert(mOffset % factor == 0);
        return OffsetPtr { mOffset / factor };
    }

private:
    uintptr_t mOffset = std::numeric_limits<uintptr_t>::max();
};

}