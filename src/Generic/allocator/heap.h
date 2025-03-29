#pragma once

#include "../bits/array.h"
#include "concepts.h"

namespace Engine {

template <Allocator Parent, typename BlockSize = TaggedPlaceholder<AllocatorAutoSizeTag, 0>, typename BlockCount = void>
struct HeapAllocator {

    static constexpr size_t blockSize = BlockSize::value;
    static constexpr size_t blockCount = BlockCount::value;

    static constexpr size_t goodSize = blockSize;

    HeapAllocator(Parent parent = {})
        : mParent(std::forward<Parent>(parent))
    {
    }

    ~HeapAllocator()
    {
        if (mParentMemory.mAddress)
            mParent.deallocate(mParentMemory);
    }

    Block allocate(size_t size)
    {
        if (!mParentMemory.mAddress)
            mParentMemory = mParent.allocate(blockSize * blockCount);

        auto result = std::ranges::search_n(mBlockState, (size - 1) / blockSize + 1, false);
        if (result.begin() == mBlockState.end())
            return {};

        for (auto block : result) {
            block = true;
        }

        return {
            static_cast<std::byte *>(mParentMemory.mAddress) + std::distance(mBlockState.begin(), result.begin()) * blockSize,
            ((size - 1) / blockSize + 1) * blockSize
        };
    }

    void deallocate(Block block)
    {
        size_t blockCount = (block.mSize - 1) / blockSize + 1;
        size_t distance = static_cast<std::byte *>(block.mAddress) - static_cast<std::byte *>(mParentMemory.mAddress);
        assert(distance % blockSize == 0);
        size_t blockIndex = distance / blockSize;
        std::fill(mBlockState.begin() + blockIndex, mBlockState.begin() + blockIndex + blockCount, false);
    }

    void deallocateAll() {
        if (mParentMemory.mAddress) {
            mParent.deallocate(mParentMemory);
            mParentMemory = {};
        }
        mBlockState = {};
    }

private:
    Parent mParent;
    Block mParentMemory;
    BitArray<blockCount> mBlockState;
};

template <Allocator Parent, typename BlockSize>
struct HeapAllocator<Parent, BlockSize, void> : HeapAllocator<Parent, BlockSize, std::integral_constant<size_t, std::remove_reference_t<Parent>::goodSize / BlockSize::value>> {
};
}