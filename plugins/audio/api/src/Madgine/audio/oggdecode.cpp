#include "../audiolib.h"

#include "oggdecode.h"

#include "Generic/stream.h"

#include "oggdecodebuf.h"

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