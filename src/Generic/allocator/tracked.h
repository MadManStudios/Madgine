#pragma once

#include "concepts.h"

namespace Engine {

template <typename Parent>
struct TrackedAllocator {

    static constexpr size_t goodSize = std::remove_reference_t<Parent>::goodSize;

    template <typename... Args>
    TrackedAllocator(Args &&...args)
        : mParent(std::forward<Args>(args)...)
    {
    }

    ~TrackedAllocator()
    {
    }

    Block allocate(size_t size, size_t alignment = 1)
    {
        Block b = mParent.allocate(size, alignment);
        mTotalBytes += b.mSize;
        return b;
    }

    void deallocate(Block block)
    {
        mTotalBytes -= block.mSize;
        mParent.deallocate(block);
    }

    void deallocateAll()
    {
        mParent.deallocateAll();
    }

    size_t totalBytes() const
    {
        return mTotalBytes;
    }

    Parent &parent()
    {
        return mParent;
    }

private:
    Parent mParent;
    size_t mTotalBytes = 0;
};

}