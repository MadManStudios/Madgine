#include "../audiolib.h"

#include "oggdecodebuf.h"

#include "audioinfo.h"

namespace Engine {
namespace Audio {

    template <auto f>
    static auto delegate2(void *destination, size_t size1, size_t size2, void *buf)
    {
        return (static_cast<OggDecodeBuf *>(buf)->*f)(destination, size1, size2);
    }

    template <auto f, typename... Args>
    static auto delegate(void *buf, Args... args)
    {
        return (static_cast<OggDecodeBuf *>(buf)->*f)(args...);
    }

    OggDecodeBuf::OggDecodeBuf()
    {
    }

    OggDecodeBuf::~OggDecodeBuf()
    {
        close();
    }

    void OggDecodeBuf::close()
    {
        if (mBase) {
            setg(nullptr, nullptr, nullptr);
            ov_clear(&mFile);
            mBase.reset();
        }
    }

    bool OggDecodeBuf::open(AudioInfo &info, std::unique_ptr<std::basic_streambuf<char>> base)
    {
        close();

        mBase = std::move(base);

        ov_callbacks callbacks;
        callbacks.close_func = nullptr;
        callbacks.read_func = delegate2<&OggDecodeBuf::ogg_read>;
        callbacks.seek_func = delegate<&OggDecodeBuf::ogg_seek, int64_t, int32_t>;
        callbacks.tell_func = delegate<&OggDecodeBuf::ogg_tell>;

        auto result = ov_open_callbacks(this, &mFile, nullptr, -1, callbacks);
        if (result) {
            {
                Log::LogDummy out { Log::MessageType::ERROR_TYPE };
                out << "Vorbis Error: ";
                switch (result) {
                    CONSTANT_CASE(OV_EREAD, out)
                    CONSTANT_CASE(OV_ENOTVORBIS, out)
                    CONSTANT_CASE(OV_EVERSION, out)
                    CONSTANT_CASE(OV_EBADHEADER, out)
                    CONSTANT_CASE(OV_EFAULT, out)
                default:
                    out << "UNKNOWN_ERROR";
                }
            }
            mBase.reset();
            return false;
        }

        vorbis_info *vorbisInfo = ov_info(&mFile, -1);

        info.mChannels = vorbisInfo->channels;
        info.mBitsPerSample = 16; //?
        info.mSampleRate = vorbisInfo->rate;
        info.mDuration = std::chrono::microseconds { static_cast<long long>(ov_time_total(&mFile, -1) * 1000000.0f) };
        info.mSampleCount = ov_pcm_total(&mFile, -1);

        return true;
    }

    OggDecodeBuf::pos_type OggDecodeBuf::seekoff(off_type off, std::ios_base::seekdir dir, std::ios_base::openmode mode)
    {
        LOG("TODO: OggDecodeBuf");
        return pos_type(off_type(-1));
    }

    OggDecodeBuf::pos_type OggDecodeBuf::seekpos(pos_type pos, std::ios_base::openmode mode)
    {
        LOG("TODO: OggDecodeBuf");
        return pos_type(off_type(-1));
    }

    OggDecodeBuf::int_type OggDecodeBuf::overflow(int c)
    {
        LOG("TODO: OggDecodeBuf");
        return traits_type::eof();
    }

    OggDecodeBuf::int_type OggDecodeBuf::underflow()
    {
        if (!mBase)
            return traits_type::eof();

        int bitstream = 0;
        auto result = ov_read(&mFile, mBuffer.data(), BUFFER_SIZE, 0, 2, 1, &bitstream);
        if (result < 0)
            std::terminate();
        if (result == 0)
            return traits_type::eof();
        setg(mBuffer.data(), mBuffer.data(), mBuffer.data() + result);
        return static_cast<unsigned char>(mBuffer[0]);
    }

    int OggDecodeBuf::sync()
    {
        if (mBase)
            mBase->pubsync();
        return 0;
    }

    size_t OggDecodeBuf::ogg_read(void *destination, size_t size1, size_t size2)
    {
        return mBase->sgetn(reinterpret_cast<char*>(destination), size1 * size2);
    }

    int32_t OggDecodeBuf::ogg_seek(int64_t to, int32_t type)
    {
        std::ios::seekdir dir;
        switch (type) {
        case SEEK_CUR:
            dir = std::ios_base::cur;
            break;
        case SEEK_END:
            dir = std::ios_base::end;
            break;
        case SEEK_SET:
            dir = std::ios_base::beg;
            break;
        default:
            std::terminate();
        }

        return mBase->pubseekoff(to, dir);
    }

    long int OggDecodeBuf::ogg_tell()
    {
        return mBase->pubseekoff(0, std::ios_base::cur, std::ios_base::in);
    }

}
}