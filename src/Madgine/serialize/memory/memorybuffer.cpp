#include "memorylib.h"

#include "memorybuffer.h"

namespace Engine {
namespace Memory {
    MemoryWriteBuffer::MemoryWriteBuffer(WritableByteBuffer buffer)
        : mWriteBuffer(std::move(buffer))
    {
        setp(static_cast<char*>(buffer.mData), static_cast<char*>(buffer.mData) + buffer.mSize);
    }

    MemoryReadBuffer::MemoryReadBuffer(ByteBuffer buffer)
        : mReadBuffer(std::move(buffer))
    {
        setg(
            const_cast<char *>(static_cast<const char *>(mReadBuffer.mData)),
            const_cast<char *>(static_cast<const char *>(mReadBuffer.mData)),
            const_cast<char *>(static_cast<const char *>(mReadBuffer.mData) + mReadBuffer.mSize));
    }

    MemoryWriteBuffer::MemoryWriteBuffer(MemoryWriteBuffer &&other) noexcept
        : std::basic_streambuf<char>(std::move(other))
        , mWriteBuffer(std::move(other.mWriteBuffer))
    {
    }

    MemoryReadBuffer::MemoryReadBuffer(MemoryReadBuffer &&other) noexcept
        : std::basic_streambuf<char>(std::move(other))
        , mReadBuffer(std::move(other.mReadBuffer))
    {
    }

    MemoryWriteBuffer::~MemoryWriteBuffer()
    {
    }

    MemoryReadBuffer::~MemoryReadBuffer()
    {
    }

    MemoryWriteBuffer::int_type MemoryWriteBuffer::overflow(int c)
    {
        size_t oldSize = mWriteBuffer.mSize;
        size_t newSize = 3 * oldSize / 2;
        if (newSize <= oldSize) {
            newSize = 16;
            assert(newSize > oldSize);
        }
        std::vector<char> newBuffer(newSize);
        std::memcpy(newBuffer.data(), mWriteBuffer.mData, oldSize);
        char *data = newBuffer.data();
        data[oldSize] = c;
        mWriteBuffer = std::move(newBuffer);
        setp(data + oldSize + 1, data + newSize);
        return c;
    }

    MemoryReadBuffer::int_type MemoryReadBuffer::underflow()
    {
        return traits_type::eof();
    }

}
}
