#pragma once

#include "Generic/bytebuffer.h"

#include "audioinfo.h"

namespace Engine {
namespace Audio {

    struct AudioBuffer {
        AudioInfo mInfo;
        ByteBuffer mBuffer;
    };

}
}