#pragma once

#include <vorbis/vorbisfile.h>

namespace Engine {
namespace Audio {

    struct OggDecodeBuf : std::basic_streambuf<char> {
        OggDecodeBuf();
        ~OggDecodeBuf();

        void close();
        bool open(AudioInfo &info, std::unique_ptr<std::basic_streambuf<char>> base);

        static const constexpr size_t BUFFER_SIZE = 1024;

    protected:

        pos_type seekoff(off_type off, std::ios_base::seekdir dir,
            std::ios_base::openmode mode = std::ios_base::in) override;
        pos_type seekpos(pos_type pos,
            std::ios_base::openmode mode = std::ios_base::in) override;

        int_type overflow(int c = EOF) override;
        int_type underflow() override;

        int sync() override;

        size_t ogg_read(void *destination, size_t size1, size_t size2);
        int32_t ogg_seek(int64_t to, int32_t type);
        long int ogg_tell();

    private:
        std::unique_ptr<std::basic_streambuf<char>> mBase;

        std::array<char, BUFFER_SIZE> mBuffer;

        OggVorbis_File mFile;
    };

}
}