#include "../audiolib.h"

#include "oggdecode.h"

#include "oggdecodebuf.h"

#include "Generic/stream.h"

namespace Engine {
namespace Audio {

    Stream DecodeOggFile(AudioInfo &info, Stream &&file)
    {
        std::unique_ptr<OggDecodeBuf> buf = std::make_unique<OggDecodeBuf>();
        if (!buf->open(info, file.release()))
            return {};
        return { std::move(buf) };
    }

}
}