#pragma once

#include "../replace.h"
#include "concepts.h"
#include "../delayedconstruct.h"

namespace Engine {

template <typename Parent, typename BucketStrategy>
struct BucketAllocatorImpl {

    template <typename... Args>
    BucketAllocatorImpl(Args &&...args)
        : BucketAllocatorImpl(typename BucketStrategy::sizes {}, std::forward<Args>(args)...)
    {
    }

    Block allocate(size_t size, size_t alignment = 1)
    {
        return TupleUnpacker::select(
            mBuckets, [size, alignment](auto &bucket) {
                assert(alignment == 1);
                return bucket.allocate(size);
            },
            BucketStrategy::select(size));
    }

    void deallocate(Block block)
    {
        return TupleUnpacker::select(
            mBuckets, [block](auto &bucket) {
                return bucket.deallocate(block);
            },
            BucketStrategy::select(block.mSize));
    }

    void deallocateAll() {
        TupleUnpacker::forEach(mBuckets, [](auto &bucket) { bucket.deallocateAll(); });
    }

private:
    template <typename T>
    using helper = typename replace<Parent>::template tagged<AllocatorAutoSizeTag, T>;

    template <typename... Args, typename... Sizes>
    BucketAllocatorImpl(type_pack<Sizes...>, Args &&...args)
        : mBuckets { DelayedConstruct<helper<Sizes>> { [&]() { return helper<Sizes> { args... }; } }... }
    {
    }

    typename BucketStrategy::sizes::template transform<helper>::as_tuple mBuckets;
};

template <
    size_t BucketSizeBegin,
    size_t BucketSizeEnd,
    size_t BucketSizeStep,
    bool condition = (BucketSizeBegin > BucketSizeEnd)>
struct BucketStrategyLinear {

    static size_t select(size_t size)
    {
        return clamp<size_t>((size - BucketSizeBegin - 1) / BucketSizeStep, 0, sizes::size - 1);
    }

    using sizes = typename BucketStrategyLinear<BucketSizeBegin + BucketSizeStep, BucketSizeEnd, BucketSizeStep>::sizes::template prepend<std::integral_constant<size_t, BucketSizeBegin>>;
};

template <size_t BucketSizeBegin, size_t BucketSizeEnd, size_t BucketSizeStep>
struct BucketStrategyLinear<BucketSizeBegin, BucketSizeEnd, BucketSizeStep, true> {
    using sizes = type_pack<>;
};

template <size_t BucketSizeBegin, size_t BucketSizeEnd, size_t BucketSizeFactor, bool condition = (BucketSizeBegin > BucketSizeEnd)>
struct BucketStrategyLog {

    static size_t select(size_t size)
    {
        return clamp<size_t>(static_cast<size_t>(std::ceil(logf(static_cast<float>(size) / BucketSizeBegin) / logf(BucketSizeFactor))), 0, sizes::size - 1);
    }

    using sizes = typename BucketStrategyLog<BucketSizeBegin * BucketSizeFactor, BucketSizeEnd, BucketSizeFactor>::sizes::template prepend<std::integral_constant<size_t, BucketSizeBegin>>;
};

template <size_t BucketSizeBegin, size_t BucketSizeEnd, size_t BucketSizeStep>
struct BucketStrategyLog<BucketSizeBegin, BucketSizeEnd, BucketSizeStep, true> {
    using sizes = type_pack<>;
};

template <typename Parent, size_t BucketSizeBegin, size_t BucketSizeEnd, size_t BucketSizeStep>
using BucketAllocator = BucketAllocatorImpl<Parent, BucketStrategyLinear<BucketSizeBegin, BucketSizeEnd, BucketSizeStep>>;

template <typename Parent, size_t BucketSizeBegin, size_t BucketSizeEnd, size_t BucketSizeFactor>
using LogBucketAllocator = BucketAllocatorImpl<Parent, BucketStrategyLog<BucketSizeBegin, BucketSizeEnd, BucketSizeFactor>>;
}