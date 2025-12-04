#include "../../interfaceslib.h"

#if ENABLE_MEMTRACKING

#    include "statsmemoryresource.h"

namespace Engine {
namespace Debug {
    namespace Memory {
        StatsMemoryResource::StatsMemoryResource(std::pmr::memory_resource *upstream)
            : mUpstream(upstream)
        {
        }

        std::pmr::memory_resource *StatsMemoryResource::upstream_resource()
        {
            return mUpstream;
        }

        size_t StatsMemoryResource::allocation_size()
        {
            return mAllocationSize;
        }

        void *StatsMemoryResource::do_allocate(size_t _Bytes, size_t _Align)
        {
            mAllocationSize += _Bytes;
            ++mAllocationCount;
            return mUpstream->allocate(_Bytes, _Align);
        }

        void StatsMemoryResource::do_deallocate(void *_Ptr, size_t _Bytes, size_t _Align)
        {
            mAllocationSize -= _Bytes;
            --mAllocationCount;
            mUpstream->deallocate(_Ptr, _Bytes, _Align);
        }

        bool StatsMemoryResource::do_is_equal(const std::pmr::memory_resource &_That) const noexcept
        {
            return _That == *mUpstream;
        }

    }
}
}

#endif