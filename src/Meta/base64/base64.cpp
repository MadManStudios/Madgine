#include "../metalib.h"

#include "base64.h"

#include "Generic/bytebuffer.h"

#include <span>

namespace Engine {
namespace Base64 {

    static constexpr const char base64Symbols[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    static constexpr std::array<uint8_t, 256> base64Lookup = []() {
        std::array<uint8_t, 256> result;
        std::fill_n(result.begin(), 256, 255);
        //result.fill(255); //TODO: Not constexpr in current NDK
        for (uint8_t i = 0; i < 64; ++i) {
            result[base64Symbols[i]] = i;
        }
        return result;
    }();

    std::string encode(const ByteBuffer &data)
    {
        std::string result;
        result.reserve((data.mSize + 2) / 3 * 4);

        int bitCount = 0;
        uint16_t buffer = 0;
        for (size_t i = 0; i < data.mSize; ++i) {
            buffer = (buffer << 8) | static_cast<const uint8_t *>(data.mData)[i];
            bitCount += 8;
            while (bitCount >= 6) {
                bitCount -= 6;
                result += base64Symbols[(buffer >> bitCount) & 0x3F];
            }
        }

        if (bitCount > 0) {
            result += base64Symbols[(buffer << (6 - bitCount)) & 0x3F];
            bitCount -= 6;
        }

        while (bitCount < 0) {
            bitCount += 2;
            result += '=';
        }

        return result;
    }

    bool decode(ByteBuffer &b, std::string_view string)
    {
        if (string.size() % 4 != 0)
            return false;
        auto rit = string.rbegin();
        while (*rit == '=')
            ++rit;
        size_t fillCount = rit - string.rbegin();
        if (fillCount >= 3)
            return false;            
        size_t resultSize = string.size() / 4 * 3 - fillCount;
        std::unique_ptr<uint8_t[]> data = std::make_unique<uint8_t[]>(resultSize);
        uint16_t buffer;
        size_t bitCount = 0;
        size_t outIndex = 0;
        for (char c : std::span { string.data(), string.size() - fillCount }) {
            uint8_t triplet = base64Lookup[c];
            if (triplet >= 64)
                return false;
            bitCount += 6;
            buffer = (buffer << 6) | triplet;
            if (bitCount >= 8) {
                bitCount -= 8;
                data[outIndex++] = buffer >> bitCount;
            }
        }
        if (outIndex < resultSize)
            data[outIndex++] = buffer << (8 - bitCount);

        assert(outIndex == resultSize);

        b = { std::move(data), resultSize };
        return true;
    }

}
}