#pragma once

#include "concepts.h"
#include "../align.h"

namespace Engine {

template <typename Parent>
struct FixedAllocator {

    static constexpr size_t goodSize = std::remove_reference_t<Parent>::goodSize;

    template <typename... Args>
    FixedAllocator(Args &&...args)
        : mParent(std::forward<Args>(args)...)
    {
    }

    ~FixedAllocator()
    {
        if (mBlock.mAddress)
            mParent.deallocate(mBlock);
    }

    Block allocate(size_t size, size_t alignment)
    {
        if (!mBlock.mAddress)
            mBlock = mParent.allocate(size, alignment);
        else {
            assert(alignTo(mBlock.mAddress, alignment) == mBlock.mAddress);
        }
        return mBlock;
    }

    void deallocate(Block block)
    {
    }

    void deallocateAll()
    {
        if (mBlock.mAddress) {
            mParent.deallocate(mBlock);
            mBlock = {};
        }
    }

    Block &getBlock()
    {
        return mBlock;
    }

private:
    Parent mParent;
    Block mBlock;
};

}