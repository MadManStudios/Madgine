#pragma once

#include "audioinfo.h"
#include "Generic/bytebuffer.h"

namespace Engine {
namespace Audio {

    struct AudioBuffer {
        AudioInfo mInfo;
        ByteBuffer mBuffer;
    };

}
}