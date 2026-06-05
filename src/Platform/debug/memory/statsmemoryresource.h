#pragma once

#if ENABLE_MEMTRACKING

namespace Engine {
namespace Debug {
    namespace Memory {

        struct StatsMemoryResource : std::pmr::memory_resource {

            StatsMemoryResource(std::pmr::memory_resource *upstream = std::pmr::get_default_resource());

            std::pmr::memory_resource *upstream_resource();

            size_t allocation_size();

        protected:
            void *do_allocate(size_t _Bytes, size_t _Align) override;
            void do_deallocate(void *_Ptr, size_t _Bytes, size_t _Align) override;
            bool do_is_equal(const std::pmr::memory_resource &_That) const noexcept override;

        private:
            size_t mAllocationCount = 0;
            size_t mAllocationSize = 0;

            std::pmr::memory_resource *mUpstream;
        };

    }
}
}

#endif