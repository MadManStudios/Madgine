#pragma once

namespace Engine {
namespace Render {

    template <typename T>
    struct GPUPtr {

        GPUPtr() = default;
        template <typename U>
        requires std::convertible_to<U *, T *>
        GPUPtr(GPUPtr<U> other)
            : mOffset(other.mOffset)
            , mBuffer(other.mBuffer)
        {
        }
        template <typename U>
        explicit GPUPtr(GPUPtr<U> other)
            : mOffset(other.mOffset)
            , mBuffer(other.mBuffer)
        {
        }

        explicit GPUPtr(void *encoded)
        {
            uintptr_t value = reinterpret_cast<uintptr_t>(encoded);
            if constexpr (sizeof(uintptr_t) == 8) {
                mBuffer = value >> 32;
                mOffset = value;
            } else {
                mBuffer = value >> 24;
                mOffset = value & ((1 << 24) - 1);
            }
        }

        void *encode() const
        {
            uintptr_t value;
            if constexpr (sizeof(uintptr_t) == 8) {
                value = (static_cast<uint64_t>(mBuffer) << 32) | mOffset;
            } else {
                value = (static_cast<uint32_t>(mBuffer) << 24) | (mOffset & ((1 << 24) - 1));
            }
            return reinterpret_cast<void *>(value);
        }

        uint32_t mOffset = 0;
        IndexType<uint32_t, 0> mBuffer;
    };

    template <typename T>
    using GPUArrayPtr = GPUPtr<T[]>;

}
}