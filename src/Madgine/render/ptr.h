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

        uint32_t mOffset = 0;
        IndexType<uint32_t, 0> mBuffer;
    };

    template <typename T>
    using GPUArrayPtr = GPUPtr<T[]>;

}
}