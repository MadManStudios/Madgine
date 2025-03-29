#pragma once

namespace Engine {

struct Block {
    void *mAddress = nullptr;
    size_t mSize = 0;
};

template <typename T>
concept Allocator = requires(T t)
{
    {
        std::remove_reference_t<T>::goodSize
        } -> std::convertible_to<size_t>;
};

struct AllocatorAutoSizeTag;
}