#pragma once

#include "Generic/flags.h"

#include "Meta/math/vector2i.h"

#include "glyph.h"

namespace Engine {
namespace Render {

    FLAGS(FontStyle,
        Bold,
        Italic,
        Light);

    struct TypeFace {
        TexturePtr mTexture;

        static constexpr size_t sFontGlyphCount = 255;
        using Font = std::array<Glyph, sFontGlyphCount>;

        std::map<FontStyle, Font> mFonts;

        int mAscender;
        int mDescender;
    };

}
}