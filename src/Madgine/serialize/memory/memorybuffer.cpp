#include "memorylib.h"

#include "memorybuffer.h"

namespace Engine {
namespace Memory {
    MemoryWriteBuffer::MemoryWriteBuffer(WritableByteBuffer &buffer)
        : mWriteBuffer(buffer)
    {
        setp(static_cast<char *>(buffer.mData), static_cast<char *>(buffer.mData) + buffer.mSize);
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
        , mWriteBuffer(other.mWriteBuffer)
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

    MemoryReadBuffer::pos_type MemoryReadBuffer::seekoff(off_type off, std::ios_base::seekdir dir,
        std::ios_base::openmode mode)
    {
        assert(mode & std::ios_base::in);

        switch (dir) {
        case std::ios_base::beg:
            if (eback() + off > egptr())
                return pos_type(off_type(-1));
            setg(eback(), eback() + off, egptr());
            break;
        case std::ios_base::cur:
            if (gptr() + off < eback() || gptr() + off > egptr())
                return pos_type(off_type(-1));
            setg(eback(), gptr() + off, egptr());
            break;
        case std::ios_base::end:
            if (egptr() + off < eback())
                return pos_type(off_type(-1));
            setg(eback(), egptr() + off, egptr());
            break;
        default:
            std::terminate();
        }

        return pos_type(off_type(gptr() - eback()));
    }

    MemoryReadBuffer::pos_type MemoryReadBuffer::seekpos(pos_type pos,
        std::ios_base::openmode mode)
    {
        assert(mode & std::ios_base::in);

        setg(eback(), eback() + pos, egptr());

        return pos_type(off_type(pos));
    }

}
}
