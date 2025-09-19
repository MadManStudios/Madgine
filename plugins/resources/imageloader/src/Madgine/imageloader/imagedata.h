#pragma once

#include "Meta/math/vector2i.h"
#include "Generic/bytebuffer.h"

namespace Engine {
namespace Resources {

    struct ImageData {
        ByteBuffer mBuffer;
        int mChannels;
        Vector2i mSize;
    };

}
}