#include "../../renderlib.h"

#include "font.h"

#include "Meta/serialize/serializetable_impl.h"

#include "fontloader.h"

SERIALIZETABLE_BEGIN(Engine::Render::TypeFace::Font)
    FIELD(mScalableFont)
    FIELD(mBitmapFonts)
SERIALIZETABLE_END(Engine::Render::TypeFace::Font)

namespace Engine {
namespace Render {

    const TypeFace::GlyphSet &TypeFace::Font::set(float fontSize, float *outScale) const
    {
        if (fontSize <= TypeFace::sMaxBitmapFontSize) {
            size_t size = std::max(TypeFace::sMinBitmapFontSize, static_cast<size_t>(fontSize));
            if (outScale)
                *outScale = 1.0f;
            return mBitmapFonts[size - sMinBitmapFontSize];
        } else {
            if (outScale)
                *outScale = fontSize / FontLoader::sFontSize;
            return mScalableFont;
        }
    }

}
}
