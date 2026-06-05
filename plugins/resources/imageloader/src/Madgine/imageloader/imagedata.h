#pragma once

#include "Generic/bytebuffer.h"

#include "Meta/math/vector2i.h"

namespace Engine {
namespace Resources {

    struct ImageData {
        Memory::ByteBuffer mBuffer;
        int mChannels;
        Math::Vector2i mSize;
    };

}
}