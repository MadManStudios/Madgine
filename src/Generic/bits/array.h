#pragma once

#include "types.h"

namespace Engine {

template <size_t Count, size_t MemberSize = 1, typename type = MinimalHoldingUIntType_t<MemberSize>>
struct BitArray {

    static constexpr uint64_t Mask = (1 << MemberSize) - 1;
    static constexpr size_t InternalArraySize = (Count * MemberSize - 1) / 8 + 1;
    static constexpr size_t FreeBitCount = InternalArraySize * 8 - Count * MemberSize;
    static constexpr uint8_t FreeBitMask = (1 << FreeBitCount) - 1;
    
    constexpr BitArray() = default;

    static constexpr uint64_t readPtrAsUInt64(const uint8_t *ptr, size_t offset)
    {
        uint64_t result = 0;
        size_t dist = (offset + MemberSize - (offset / 8) * 8 - 1) / 8 + 1;
        for (int i = dist - 1; i >= 0; --i) {
            result <<= 8;
            result += ptr[offset / 8 + i];
        }
        return result;
    }

    static constexpr void writePtrAsUInt64(uint8_t *ptr, size_t offset, uint64_t value)
    {
        size_t dist = (offset + MemberSize - (offset / 8) * 8 - 1) / 8 + 1;
        for (int i = 0; i < dist; ++i) {
            ptr[offset / 8 + i] = value;
            value >>= 8;
        }
    }

    struct const_reference {

        constexpr operator type() const
        {
            uint64_t value = readPtrAsUInt64(mPointer, mOffset) >> (mOffset % 8);
            value &= Mask;
            return static_cast<type>(value);
        }

        const uint8_t *mPointer;
        size_t mOffset;
    };

    struct reference {
        constexpr reference operator=(type v)
        {
            uint64_t newMask = Mask << (mOffset % 8);
            uint64_t newValue = static_cast<uint64_t>(v);
            newValue <<= (mOffset % 8);
            writePtrAsUInt64(mPointer, mOffset, (readPtrAsUInt64(mPointer, mOffset) & ~newMask) | newValue);
            return *this;
        }

        constexpr operator type() const
        {
            uint64_t value = readPtrAsUInt64(mPointer, mOffset) >> (mOffset % 8);
            value &= Mask;
            return static_cast<type>(value);
        }

        uint8_t *mPointer;
        size_t mOffset;
    };

    struct const_pointer {

        using difference_type = ptrdiff_t;
        using value_type = type;

        constexpr const_reference operator*() const
        {
            return { mPointer, mOffset };
        }

        constexpr const_pointer &operator++()
        {
            mOffset += MemberSize;
            return *this;
        }

        constexpr const_pointer operator++(int)
        {
            const_pointer other = *this;
            ++other;
            return other;
        }

        constexpr bool operator!=(const const_pointer &other) const
        {
            assert(mPointer == other.mPointer);
            return mOffset != other.mOffset;
        }

        constexpr bool operator==(const const_pointer &other) const
        {
            assert(mPointer == other.mPointer);
            return mOffset == other.mOffset;
        }

        const uint8_t *mPointer;
        size_t mOffset;
    };

    struct pointer {

        using difference_type = ptrdiff_t;
        using value_type = type;
        using iterator_category = std::forward_iterator_tag;

        constexpr reference operator*() const
        {
            return { mPointer, mOffset };
        }

        constexpr pointer &operator++()
        {
            mOffset += MemberSize;
            return *this;
        }

        constexpr pointer operator++(int)
        {
            pointer other = *this;
            ++other;
            return other;
        }

        constexpr pointer& operator+=(size_t diff) {
            mOffset += diff;
            return *this;
        }

        constexpr pointer operator+(size_t diff) const {
            pointer other = *this;
            other += diff;
            return other;
        }

        constexpr bool operator!=(const pointer &other) const
        {
            assert(mPointer == other.mPointer);
            return mOffset != other.mOffset;
        }

        constexpr bool operator==(const pointer &other) const
        {
            assert(mPointer == other.mPointer);
            return mOffset == other.mOffset;
        }

        uint8_t *mPointer;
        size_t mOffset;
    };

    constexpr reference operator[](size_t i)
    {
        assert(i < Count);
        return { mData, FreeBitCount + i * MemberSize };
    }

    constexpr const_reference operator[](size_t i) const
    {
        assert(i < Count);
        return { mData, FreeBitCount + i * MemberSize };
    }

    constexpr const_pointer begin() const
    {
        return { mData, FreeBitCount };
    }

    constexpr const_pointer end() const
    {
        return { mData, FreeBitCount + Count * MemberSize };
    }

    constexpr pointer begin()
    {
        return { mData, FreeBitCount };
    }

    constexpr pointer end()
    {
        return { mData, FreeBitCount + Count * MemberSize };
    }

    constexpr bool operator==(const BitArray &other) const
    {
        if ((mData[0] & ~FreeBitMask) != (other.mData[0] & ~FreeBitMask))
            return false;
        for (size_t i = 1; i < InternalArraySize; ++i) {
            if (mData[i] != other.mData[i])
                return false;
        }
        return true;
    }

    constexpr bool operator!=(const BitArray &other) const
    {
        return !(*this == other);
    }

    constexpr std::strong_ordering operator<=>(const BitArray &other) const
    {
        if (auto cmp = (mData[0] & ~FreeBitMask) <=> (other.mData[0] & ~FreeBitMask); cmp != 0)
            return cmp;
        for (size_t i = 1; i < InternalArraySize; ++i) {
            auto cmp = mData[i] <=> other.mData[i];
            if (cmp != 0)
                return cmp;
        }
        return std::strong_ordering::equal;
    }

private:
    uint8_t mData[InternalArraySize] = {};
};

}