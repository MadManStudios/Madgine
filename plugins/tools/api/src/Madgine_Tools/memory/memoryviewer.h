#pragma once

#if ENABLE_MEMTRACKING

#    include "../toolbase.h"
#    include "../toolscollector.h"
#    include "Interfaces/debug/memory/memory.h"
#    include "Interfaces/debug/stacktrace.h"

namespace Engine {
namespace Tools {
    using Item = const std::pair<const Debug::FullStackTrace, Debug::Memory::TracedAllocationData>;

    struct BlockData {
        BlockData(Item *front = nullptr, Item **back = nullptr, BlockData *next = nullptr, size_t size = 0)
            : mFront(front)
            , mBack(back)
            , mNext(next)
            , mSize(size)
        {
        }
        Item *mFront = nullptr;
        Item **mBack = nullptr;
        BlockData *mNext = nullptr;
        size_t mSize = 0;
        BlockData *mFirstChild = nullptr;
    };

    struct MemoryBlock {
        MemoryBlock(Item *source = nullptr)
            : mSources(source)
        {
        }
        BlockData mData;
        std::list<BlockData> mLeafs;
        std::unordered_map<void *, MemoryBlock> mChildren;
        Item *mSources = nullptr;
    };

    struct MemoryViewer : Tool<MemoryViewer> {
        enum SortMode {
            NO_SORTING,
            METHODNAME_SORTING,
            ADDRESS_SORTING,
            FILE_SORTING,
            BLOCK_SIZE_SORTING,
            SIZE_SORTING
        };

        MemoryViewer(ImRoot &root);

        virtual void render() override;

        std::string_view key() const override;

    private:
        void traceDraw(const std::pmr::vector<Engine::Debug::TraceBack> &data, size_t size, size_t blockSize, int depth);
        bool traceLevel(const Engine::Debug::TraceBack &traceback, size_t size, size_t blockSize, bool leaf);
        void drawBlock(const BlockData &block, size_t depth);

        bool drawHeader(const char *title, SortMode mode, float size = 0);

    private:
        Debug::Memory::MemoryTracker &mTracker;

        bool mSortDescending = false;
        SortMode mSortMode = NO_SORTING;

        bool mShowAddress = true, mShowFile = true;

        bool mCollapsing = false;

        MemoryBlock mRootBlock;
    };

}
}

REGISTER_TYPE(Engine::Tools::MemoryViewer)

#endif