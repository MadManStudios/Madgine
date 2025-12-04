#pragma once

#if ENABLE_MEMTRACKING

#    include "../stacktrace.h"

namespace Engine {
namespace Debug {
    namespace Memory {

        struct TracedAllocationData {
            size_t mSize = 0;
            mutable size_t mGeneration = 0;
            mutable const std::pair<const FullStackTrace, TracedAllocationData> *mNext = nullptr;
            mutable size_t mBlockSize = 0;
        };

        struct INTERFACES_EXPORT MemoryTracker {

            using StackTrace = StackTrace<64>;

            MemoryTracker();
            MemoryTracker(const MemoryTracker &) = delete;
            ~MemoryTracker();

            static MemoryTracker &getSingleton();

            size_t overhead();
            size_t totalMemory();

            const std::pmr::unordered_map<FullStackTrace, TracedAllocationData> &stacktraces();
            std::atomic<const std::pair<const FullStackTrace, TracedAllocationData> *> &linkedFront();

            void onMalloc(uintptr_t id, size_t s);
            void onFree(uintptr_t id, size_t s);

            static void *allocateUntracked(size_t size, size_t align);
            static void deallocateUntracked(void *ptr, size_t size, size_t align);

        private:
            size_t mTotalMemory = 0;
            size_t mUnknownAllocationSize = 0;

            std::atomic<const std::pair<const FullStackTrace, TracedAllocationData> *> mLinkedFront = nullptr;

            std::pmr::unordered_map<FullStackTrace, TracedAllocationData> mUnknownFullStacktraces;
            std::pmr::unordered_map<StackTrace, TracedAllocationData *> mUnknownAllocations;
            std::pmr::unordered_map<uintptr_t, TracedAllocationData *> mAllocations;
            std::pmr::unordered_map<StackTrace, TracedAllocationData *> mStacktraces;
            std::pmr::unordered_map<FullStackTrace, TracedAllocationData> mFullStacktraces;

        private:
        };

    }
}
}

#endif
