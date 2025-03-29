#pragma once

namespace Engine {

/**
	 * @brief Aligns size to the specified alignment, rounding up
	 * @param size input size
	 * @param alignment needed alignment
	 * @return the aligned size
	*/
constexpr size_t alignTo(size_t size, size_t alignment)
{
    return size + alignment - (((size - 1) % alignment) + 1);
}

inline void *alignTo(void *ptr, size_t alignment)
{
    std::byte *bytePtr = static_cast<std::byte *>(ptr);
    return bytePtr + alignment - (((reinterpret_cast<uintptr_t>(ptr) - 1) % alignment) + 1);
}

}