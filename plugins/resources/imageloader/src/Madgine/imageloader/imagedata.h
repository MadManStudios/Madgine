#pragma once

#include "Generic/bytebuffer.h"

#include "Meta/math/vector2i.h"

namespace Engine {
namespace Resources {

    struct ImageData {
        ByteBuffer mBuffer;
        int mChannels;
        Vector2i mSize;
    };

}
}