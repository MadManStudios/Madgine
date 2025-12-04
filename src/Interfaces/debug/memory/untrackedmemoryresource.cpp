#include "../../interfaceslib.h"

#if ENABLE_MEMTRACKING

#    include "memory.h"
#    include "untrackedmemoryresource.h"

namespace Engine {
namespace Debug {
    namespace Memory {

        StatsMemoryResource *UntrackedMemoryResource::sInstance()
        {
            static UntrackedMemoryResource resource;
            static StatsMemoryResource stats(&resource);
            return &stats;
        }

        void *UntrackedMemoryResource::do_allocate(size_t _Bytes, size_t _Align)
        {
            return MemoryTracker::allocateUntracked(_Bytes, _Align);
        }

        void UntrackedMemoryResource::do_deallocate(void *_Ptr, size_t _Bytes, size_t _Align)
        {
            MemoryTracker::deallocateUntracked(_Ptr, _Bytes, _Align);
        }

        bool UntrackedMemoryResource::do_is_equal(const std::pmr::memory_resource &_That) const noexcept
        {
            return true;
        }

    }
}
}

#endif