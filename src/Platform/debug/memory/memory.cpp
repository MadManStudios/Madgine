#include "../../platformlib.h"

#if ENABLE_MEMTRACKING

#    include "memory.h"
#    include "statsmemoryresource.h"
#    include "untrackedmemoryresource.h"

#    if WINDOWS
#        define NOMINMAX
#        include <Windows.h>

#        define nNoMansLandSize 4

typedef struct _CrtMemBlockHeader {
    struct _CrtMemBlockHeader *pBlockHeaderNext;
    struct _CrtMemBlockHeader *pBlockHeaderPrev;
    char *szFileName;
    int nLine;
#        ifdef _WIN64
    /* These items are reversed on Win64 to eliminate gaps in the struct
     * and ensure that sizeof(struct)%16 == 0, so 16-byte alignment is
     * maintained in the debug heap.
     */
    int nBlockUse;
    size_t nDataSize;
#        else /* _WIN64 */
    size_t nDataSize;
    int nBlockUse;
#        endif /* _WIN64 */
    long lRequest;
    unsigned char gap[nNoMansLandSize];
    /* followed by:
     *  unsigned char           data[nDataSize];
     *  unsigned char           anotherGap[nNoMansLandSize];
     */
} _CrtMemBlockHeader;

#    elif UNIX

#        include <iostream>
#        include <malloc.h>

#    endif

namespace Engine {
namespace Debug {
    namespace Memory {

        static MemoryTracker *sSingleton;

        MemoryTracker &MemoryTracker::getSingleton()
        {
            assert(sSingleton);
            return *sSingleton;
        }

        size_t MemoryTracker::overhead()
        {
            return UntrackedMemoryResource::sInstance()->allocation_size();
        }

        size_t MemoryTracker::totalMemory()
        {
            return mTotalMemory;
        }

        const std::pmr::unordered_map<FullStackTrace, TracedAllocationData> &MemoryTracker::stacktraces()
        {
            return mFullStacktraces;
        }

        std::atomic<const std::pair<const FullStackTrace, TracedAllocationData> *> &MemoryTracker::linkedFront()
        {
            return mLinkedFront;
        }

#    if WINDOWS

        int (*sOldHook)(int, void *, size_t, int, long, const unsigned char *, int) = nullptr;

        static int win32Hook(int allocType, void *userData, size_t size, int blockType, long requestNumber, const unsigned char *filename, int lineNumber)
        {
            _CrtMemBlockHeader *header;
            switch (allocType) {
            case _HOOK_ALLOC:
                sSingleton->onMalloc(requestNumber, size);
                break;
            case _HOOK_FREE:
                header = ((_CrtMemBlockHeader *)userData) - 1;
                sSingleton->onFree(header->lRequest, header->nDataSize);
                break;
            case _HOOK_REALLOC:
                header = ((_CrtMemBlockHeader *)userData) - 1;
                sSingleton->onFree(header->lRequest, header->nDataSize);
                sSingleton->onMalloc(requestNumber, size);
                break;
            }

            return true;
        }

        void *MemoryTracker::allocateUntracked(size_t size, size_t align)
        {
            static HANDLE heap = GetProcessHeap();
            return HeapAlloc(heap, HEAP_GENERATE_EXCEPTIONS, size);
        }

        void MemoryTracker::deallocateUntracked(void *ptr, size_t size, size_t align)
        {
            static HANDLE heap = GetProcessHeap();
            auto result = HeapFree(heap, 0, ptr);
            assert(result);
        }

#    elif LINUX || ANDROID

        void *(*sOldMallocHook)(size_t, const void *) = nullptr;
        void *(*sOldReallocHook)(void *, size_t, const void *) = nullptr;
        void (*sOldFreeHook)(void *, const void *) = nullptr;

        static void *linuxMallocHook(size_t size, const void *);
        static void *linuxReallocHook(void *ptr, size_t size, const void *);
        static void linuxFreeHook(void *ptr, const void *);

        static bool pushUntracked()
        {
            if (__malloc_hook == sOldMallocHook)
                return false;
            __malloc_hook = sOldMallocHook;
            __realloc_hook = sOldReallocHook;
            __free_hook = sOldFreeHook;
            return true;
        }

        static void popUntracked(bool b)
        {
            if (b) {
                __malloc_hook = linuxMallocHook;
                __realloc_hook = linuxReallocHook;
                __free_hook = linuxFreeHook;
            }
        }

        void *MemoryTracker::allocateUntracked(size_t size, size_t align)
        {
            bool b = pushUntracked();
            void *ptr = malloc(size);
            popUntracked(b);
            return ptr;
        }

        void MemoryTracker::deallocateUntracked(void *ptr, size_t size, size_t align)
        {
            bool b = pushUntracked();
            free(ptr);
            popUntracked(b);
        }

        void *reallocUntracked(void *ptr, size_t size)
        {
            bool b = pushUntracked();
            void *result = realloc(ptr, size);
            popUntracked(b);
            return result;
        }

        static void *linuxMallocHook(size_t size, const void *)
        {
            bool b = pushUntracked();
            void *ptr = malloc(size);
            sSingleton->onMalloc(reinterpret_cast<uintptr_t>(ptr), size);
            popUntracked(b);
            return ptr;
        }

        static void *linuxReallocHook(void *ptr, size_t size, const void *)
        {
            bool b = pushUntracked();
            sSingleton->onFree(reinterpret_cast<uintptr_t>(ptr), size);
            void *result = realloc(ptr, size);
            sSingleton->onMalloc(reinterpret_cast<uintptr_t>(result), size);
            popUntracked(b);
            return result;
        }

        static void linuxFreeHook(void *ptr, const void *)
        {
            bool b = pushUntracked();
            // TODO (0)
            sSingleton->onFree(reinterpret_cast<uintptr_t>(ptr), 0);
            free(ptr);
            popUntracked(b);
        }

#    else
#        error "Unsupported Platform!"
#    endif

        MemoryTracker::MemoryTracker()
            : mUnknownFullStacktraces(UntrackedMemoryResource::sInstance())
            , mUnknownAllocations(UntrackedMemoryResource::sInstance())
            , mAllocations(UntrackedMemoryResource::sInstance())
            , mStacktraces(UntrackedMemoryResource::sInstance())
            , mFullStacktraces(UntrackedMemoryResource::sInstance())
        {
            assert(!sSingleton);
            sSingleton = this;

#    if WINDOWS
            sOldHook = _CrtSetAllocHook(&win32Hook);
#    elif UNIX
            sOldMallocHook = __malloc_hook;
            sOldReallocHook = __realloc_hook;
            sOldFreeHook = __free_hook;
            __malloc_hook = linuxMallocHook;
            __realloc_hook = linuxReallocHook;
            __free_hook = linuxFreeHook;
#    endif
        }

        MemoryTracker::~MemoryTracker()
        {
#    if WINDOWS
            _CrtSetAllocHook(sOldHook);
#    elif UNIX
            __malloc_hook = sOldMallocHook;
            __realloc_hook = sOldReallocHook;
            __free_hook = sOldFreeHook;
#    endif

            sSingleton = nullptr;

#    if UNIX
#        define OutputDebugString(msg) std::cout << msg
#    endif

            OutputDebugString("-------- Madgine Memory Tracker Report --------\n");

            for (auto &track : mFullStacktraces) {
                if (track.second.mSize == 0)
                    continue;
                for (const TraceBack &trace : track.first) {
                    OutputDebugString((std::to_string(trace) + '\n').c_str());
                }
                OutputDebugString(("Current Size: "s + std::to_string(track.second.mSize) + '\n').c_str());
            }

            OutputDebugString("-------- Unknown Deallocations --------\n");

            for (auto &track : mUnknownFullStacktraces) {
                if (track.second.mSize == 0)
                    continue;
                for (const TraceBack &trace : track.first) {
                    OutputDebugString((std::to_string(trace) + '\n').c_str());
                }
                OutputDebugString(("Total Size: "s + std::to_string(track.second.mSize) + '\n').c_str());
            }

            OutputDebugString("-------- Madgine Memory Tracker Summary --------\n");

            OutputDebugString(("Total Leak: "s + std::to_string(mTotalMemory) + '\n').c_str());
            OutputDebugString(("Total Overhead: "s + std::to_string(overhead()) + '\n').c_str());

            OutputDebugString("-------- Madgine Memory Tracker End --------\n");
        }

        void MemoryTracker::onMalloc(uintptr_t id, size_t s)
        {
            mTotalMemory += s;

            auto pib = mStacktraces.try_emplace(StackTrace::getCurrent(6));

            if (pib.second) {
                auto pib2 = mFullStacktraces.try_emplace(pib.first->first.calculateReadable());
                pib.first->second = &pib2.first->second;
                if (pib2.second) {
                    pib2.first->second.mNext = mLinkedFront;
                    mLinkedFront = &*pib2.first;
                }
            }

            pib.first->second->mSize += s;
            pib.first->second->mGeneration = std::min(pib.first->second->mGeneration, size_t(1));
            pib.first->second->mBlockSize = s;

            auto pib2 = mAllocations.try_emplace(id, pib.first->second);
            assert(pib2.second);
        }

        void MemoryTracker::onFree(uintptr_t id, size_t s)
        {
            auto it = mAllocations.find(id);
            if (it != mAllocations.end()) {

                mTotalMemory -= s;

                assert(it != mAllocations.end());

                it->second->mSize -= s;
                it->second->mGeneration = std::min(it->second->mGeneration, size_t(1));

                mAllocations.erase(it);
            } else {
                mUnknownAllocationSize += s;
                auto pib = mUnknownAllocations.try_emplace(StackTrace::getCurrent(6));
                if (pib.second) {
                    auto pib2 = mUnknownFullStacktraces.try_emplace(pib.first->first.calculateReadable());
                    pib.first->second = &pib2.first->second;
                }
                pib.first->second->mSize += s;
            }
        }

    }
}
}

#endif