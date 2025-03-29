#pragma once

#include "../align.h"
#include "concepts.h"

namespace Engine {

template <typename Parent>
struct BumpAllocator {

    static constexpr size_t goodSize = Parent::goodSize;

    template <typename... Args>
    BumpAllocator(Args &&...args)
        : mParent(std::forward<Args>(args)...)
    {
    }

    Block allocate(size_t size, size_t alignment = 1)
    {
        assert(size <= Parent::goodSize);
        mOffset = alignTo(mOffset, alignment);
        if (mCurrentBlock.mSize < mOffset + size) {
            mCurrentBlock = mParent.allocate(Parent::goodSize, alignment);
            mOffset = 0;
        }
        Block result;
        result.mAddress = static_cast<std::byte *>(mCurrentBlock.mAddress) + mOffset;
        result.mSize = size;
        mOffset += size;
        return result;
    }

    void deallocate(Block block)
    {
    }

    void deallocateAll()
    {
        mParent.deallocateAll();
    }

    Parent &parent()
    {
        return mParent;
    }

private:
    Parent mParent;
    Block mCurrentBlock;
    size_t mOffset = 0;
};

}