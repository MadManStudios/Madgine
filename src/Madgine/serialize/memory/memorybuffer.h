#pragma once

#include "Generic/bytebuffer.h"

namespace Engine {
namespace Memory {
    struct MADGINE_MEMORY_SERIALIZE_EXPORT MemoryWriteBuffer : std::basic_streambuf<char> {
        MemoryWriteBuffer(WritableByteBuffer buffer);
        MemoryWriteBuffer(const MemoryWriteBuffer &) = delete;
        MemoryWriteBuffer(MemoryWriteBuffer &&other) noexcept;
        virtual ~MemoryWriteBuffer();

    protected:
        int_type overflow(int c = EOF) override;

    private:
        WritableByteBuffer mWriteBuffer;
    };

    struct MADGINE_MEMORY_SERIALIZE_EXPORT MemoryReadBuffer : std::basic_streambuf<char> {
        MemoryReadBuffer(ByteBuffer buffer);
        MemoryReadBuffer(const MemoryReadBuffer &) = delete;
        MemoryReadBuffer(MemoryReadBuffer &&other) noexcept;
        virtual ~MemoryReadBuffer();

    protected:
        int_type underflow() override;

        pos_type seekoff(off_type off, std::ios_base::seekdir way, std::ios_base::openmode which = std::ios_base::in) override;
        pos_type seekpos(pos_type sp, std::ios_base::openmode which = std::ios_base::in) override;

    private:
        ByteBuffer mReadBuffer;
    };
}
}
