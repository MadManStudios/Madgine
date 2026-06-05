#pragma once

#if ENABLE_MEMTRACKING

#    include "statsmemoryresource.h"

namespace Engine {
namespace Debug {
    namespace Memory {

        struct INTERFACES_EXPORT UntrackedMemoryResource : std::pmr::memory_resource {
            static StatsMemoryResource *sInstance();

        protected:
            void *do_allocate(size_t _Bytes, size_t _Align) override;
            void do_deallocate(void *_Ptr, size_t _Bytes, size_t _Align) override;
            bool do_is_equal(const std::pmr::memory_resource &_That) const noexcept override;
        };

    }
}
}

#endif