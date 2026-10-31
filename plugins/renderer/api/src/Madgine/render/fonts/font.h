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

        using GlyphSet = std::array<Glyph, sFontGlyphCount>;

        static constexpr size_t sMaxBitmapFontSize = 24;
        static constexpr size_t sMinBitmapFontSize = 6;
        struct MADGINE_RENDER_EXPORT Font {
            GlyphSet mScalableFont;
            std::array<GlyphSet, sMaxBitmapFontSize - sMinBitmapFontSize + 1 > mBitmapFonts;

            const GlyphSet &set(float fontSize, float *outScale = nullptr) const;
        };

        std::map<FontStyle, Font> mFonts;

        int mAscender;
        int mDescender;
    };

}
}