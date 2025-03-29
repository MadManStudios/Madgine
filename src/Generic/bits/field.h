#pragma once

#include "types.h"

namespace Engine {

template <size_t FreeBits, size_t Size, typename Type = MinimalHoldingUIntType_t<Size>>
struct BitField {
    using InnerType = MinimalHoldingUIntType_t<Size + FreeBits>;
    static constexpr InnerType Mask = (1 << Size) - 1;

    ~BitField()
    {
        *this = 0xdddddddd & Mask;
    }

    operator Type() const
    {
        return (mHolder >> FreeBits) & Mask;
    }

    BitField &operator=(Type t)
    {
        InnerType inner = t;
        assert((inner & Mask) == inner);
        mHolder = (mHolder & ~(Mask << FreeBits)) | (inner << FreeBits);
        return *this;
    }

private:
    InnerType mHolder;
};


}