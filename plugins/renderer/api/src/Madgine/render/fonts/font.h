#pragma once

#include "Meta/math/vector2i.h"

#include "glyph.h"

namespace Engine {
namespace Render {

    struct Font {
        TexturePtr mTexture;

        static constexpr size_t sFontGlyphCount = 255;
        std::array<Glyph, sFontGlyphCount> mGlyphs;

        int mAscender;
        int mDescender;
    };

}
}