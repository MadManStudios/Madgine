#pragma once

namespace Engine {

template <size_t bits>
using MinimalHoldingUIntType = std::enable_if<bits <= 64, std::conditional_t<(bits > 32), uint64_t, std::conditional_t<(bits > 16), uint32_t, std::conditional_t<(bits > 8), uint16_t, std::conditional_t<(bits > 1), uint8_t, bool>>>>>;

template <size_t bits>
using MinimalHoldingUIntType_t = typename MinimalHoldingUIntType<bits>::type;

}