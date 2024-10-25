#pragma once

#include "ptr.h"
#include "Generic/opaqueptr.h"

namespace Engine {
namespace Render {

    template <typename T>
    struct GPUBuffer {

        GPUBuffer() = default;

        template <typename U>
        requires std::convertible_to<U *, T *>
        GPUBuffer(GPUBuffer<U> other)
            : mBuffer(std::move(other.mBuffer))
            , mPtr(other.mPtr)
            , mSize(other.mSize)
        {
        }

        template <typename U>
        explicit GPUBuffer(GPUBuffer<U> other)
            : mBuffer(std::move(other.mBuffer))
            , mPtr(other.mPtr)
            , mSize(other.mSize)
        {
        }

        template <typename U>
        requires std::convertible_to<T *, U *>
        constexpr operator GPUBuffer<U> &() &
        {
            return reinterpret_cast<GPUBuffer<U> &>(*this);
        }

        constexpr operator GPUPtr<T>() const
        {
            return mPtr;
        }

        UniqueOpaquePtr mBuffer;
        GPUPtr<T> mPtr;
        size_t mSize;
    };

}
}