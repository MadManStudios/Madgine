#pragma once

#include "../containers/bits/array.h"
#include "concepts.h"

namespace Engine {
namespace Memory {

    template <Allocator Parent, typename BlockSize = TaggedPlaceholder<AllocatorAutoSizeTag, 0>, typename BlockCount = void>
    struct HeapAllocator {

        static constexpr size_t blockSize = BlockSize::value;
        static constexpr size_t blockCount = BlockCount::value;

        static constexpr size_t goodSize = blockSize;

        struct Chunk {

            Chunk(Parent &parent)
                : mParentMemory(parent.allocate(blockSize * blockCount))
            {
                assert(mParentMemory.mAddress);
            }

            ~Chunk()
            {
                assert(!mParentMemory.mAddress);
            }

            Memory::Block allocate(size_t size, size_t alignment)
            {
                if (size > blockSize * blockCount)
                    return {};

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

            void deallocate(Memory::Block block)
            {
                size_t blockCount = (block.mSize - 1) / blockSize + 1;
                size_t distance = static_cast<std::byte *>(block.mAddress) - static_cast<std::byte *>(mParentMemory.mAddress);
                assert(distance % blockSize == 0);
                size_t blockIndex = distance / blockSize;
                std::fill(mBlockState.begin() + blockIndex, mBlockState.begin() + blockIndex + blockCount, false);
            }

            void deallocateAll(Parent &parent)
            {
                parent.deallocate(mParentMemory);
                mParentMemory = {};
                mBlockState = {};
            }

            bool contains(void* p) const {
                return mParentMemory.mAddress <= p && p < static_cast<std::byte *>(mParentMemory.mAddress) + mParentMemory.mSize;
            }

            Block mParentMemory;
            Containers::BitArray<blockCount> mBlockState;
        };

        HeapAllocator(Parent parent)
            : mParent(std::forward<Parent>(parent))
        {
        }

        ~HeapAllocator()
        {
            deallocateAll();
        }

        Memory::Block allocate(size_t size, size_t alignment)
        {
            assert(alignment <= blockSize);
            assert(blockSize % alignment == 0);

            for (Chunk &chunk : mChunks) {
                Memory::Block block = chunk.allocate(size, alignment);
                if (block.mAddress)
                    return block;
            }

            return mChunks.emplace_back(mParent).allocate(size, alignment);
        }

        void deallocate(Memory::Block block)
        {
            auto it = std::ranges::find_if(mChunks, [&](Chunk &chunk) { return chunk.contains(block.mAddress); });
            assert(it != mChunks.end());
            it->deallocate(block);
        }

        void deallocateAll()
        {
            for (Chunk &chunk : mChunks) {
                chunk.deallocateAll(mParent);
            }
            mChunks.clear();
        }

    private:
        Parent mParent;
        std::list<Chunk> mChunks;
    };

    template <Allocator Parent, typename BlockSize>
    struct HeapAllocator<Parent, BlockSize, void> : HeapAllocator<Parent, BlockSize, std::integral_constant<size_t, std::remove_reference_t<Parent>::goodSize / BlockSize::value>> {
    };

}
}