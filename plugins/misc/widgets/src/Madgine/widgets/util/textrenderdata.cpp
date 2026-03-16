#include "../../widgetslib.h"

#include "textrenderdata.h"

#include "Madgine/render/texture.h"

#include "Meta/keyvalue/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

#include "widgetsrenderdata.h"

METATABLE_BEGIN_BASE(Engine::Widgets::TextRenderData, Engine::Widgets::RenderData)
    MEMBER(mFontSize)
    MEMBER(mPivot)
    MEMBER(mColor)
    PROPERTY(Font, getFont, setFont)
    MEMBER(mStyle)
    MEMBER(mShadowOffset)
METATABLE_END(Engine::Widgets::TextRenderData)

SERIALIZETABLE_INHERIT_BEGIN(Engine::Widgets::TextRenderData, Engine::Widgets::RenderData)
    FIELD(mFontSize)
    FIELD(mPivot)
    FIELD(mColor)
    ENCAPSULATED_FIELD(mFont, getFontName, setFontName)
    FIELD(mStyle)
    FIELD(mShadowOffset)
SERIALIZETABLE_END(Engine::Widgets::TextRenderData)

namespace Engine {
namespace Widgets {

    std::string_view TextRenderData::getFontName() const
    {
        return mFont.name();
    }

    void TextRenderData::setFontName(std::string_view name)
    {
        mFont.load(name);
    }

    Render::FontLoader::Resource *TextRenderData::getFont() const
    {
        return mFont.resource();
    }

    void TextRenderData::setFont(Render::FontLoader::Resource *font)
    {
        mFont = font;
    }

    bool TextRenderData::available() const
    {
        return mFont && mFont.available();
    }

    void TextRenderData::render(WidgetsRenderData &renderData, std::string_view text, Vector2 pos, Vector3 size, int cursorIndex) const
    {
        renderText(renderData, text, pos, size.xy(), mFont, mStyle, size.z * mFontSize, mColor.frame(pos, size.xy()), mPivot, mShadowOffset, cursorIndex);
    }

    void TextRenderData::renderSelection(WidgetsRenderData &renderData, std::string_view text, Vector2 pos, Vector3 size, const Atlas2::Entry &entry, int selectionStart, int selectionEnd, ColorFrame color)
    {
        renderSelection(renderData, text, pos, size.xy(), mFont, mStyle, size.z * mFontSize, mPivot, entry, selectionStart, selectionEnd, color);
    }

    float TextRenderData::calculateWidth(std::string_view text, float z)
    {
        return calculateWidth(text, mFont, mStyle, z * mFontSize);
    }

    float TextRenderData::calculateWidth(char c, float z)
    {
        return calculateWidth(c, mFont, mStyle, z * mFontSize);
    }

    float TextRenderData::calculateLineHeight(float z) const
    {
        return calculateLineHeight(mFont, z * mFontSize);
    }

    Rect2 TextRenderData::calculateBoundingBox(const Line &line, size_t lineCount, size_t lineNr, Vector2 pos, Vector3 size)
    {
        return calculateBoundingBox(line, lineCount, lineNr, pos, size.xy(), mFont, size.z * mFontSize, mPivot);
    }

    Rect2 TextRenderData::calculateBoundingBox(std::string_view text, Vector2 pos, Vector3 size)
    {
        return calculateBoundingBox(text, pos, size.xy(), mFont, mStyle, size.z * mFontSize, mPivot);
    }

    void TextRenderData::renderText(WidgetsRenderData &renderData, std::string_view text, Vector2 pos, Vector2 size, const Render::TypeFace *typeFace, Render::FontStyle style, float fontSize, ColorFrame color, Vector2 pivot, Vector2 shadowOffset, int cursorIndex)
    {
        if (text.empty() && cursorIndex == -1)
            return;

        float scale = fontSize / Render::FontLoader::sFontSize;

        float minY = typeFace->mDescender / 64.0f * scale;
        float maxY = typeFace->mAscender / 64.0f * scale;
        float fullHeight = maxY - minY;
        float fullWidth = calculateWidth(text, typeFace, style, fontSize);

        float originY = (size.y - fullHeight) * pivot.y + maxY;

        renderLine(renderData, { text.data(), text.data() + text.size(), fullWidth }, originY, pos, size, typeFace, style, fontSize, color, pivot, shadowOffset, cursorIndex);
    }

    void TextRenderData::renderLine(WidgetsRenderData &renderData, const Line &line, float originY, Vector2 pos, Vector2 size, const Render::TypeFace *typeFace, Render::FontStyle style, float fontSize, ColorFrame color, Vector2 pivot, Vector2 shadowOffset, int cursorIndex)
    {
        float scale = fontSize / Render::FontLoader::sFontSize;

        float cursorX = (size.x - line.mWidth) * pivot.x;

        bool useSmallSize = fontSize < 32;

        TextureSettings tex { typeFace->mTexture->resourceBlock(), useSmallSize ? 0 : TextureFlag_IsDistanceField };

        const Render::Glyph &ref = typeFace->mFonts.at(style)['D'];

        float cursorHeight = ref.mSize.y * scale;

        size_t baseLayer = renderData.subLayer();

        size_t index = 0;
        for (char32_t c : StringUtil::parseUTF8({line.mBegin, line.mEnd})) {

            if (index == cursorIndex) {
                const Render::Glyph &cursor = typeFace->mFonts.at(style)['|'];

                float width = 3.0f * scale;

                float startX = cursorX - 1.5f * scale;
                float startY = originY - ref.mBearing.y * scale;

                renderData.setSubLayer(baseLayer + 2);
                if (useSmallSize)
                    renderData.renderQuadUV({ pos.x + startX, pos.y + startY }, { width, cursorHeight }, color, tex, { cursor.mUV2, cursor.mSize2 }, typeFace->mTexture->size(), cursor.mFlipped2);
                else
                    renderData.renderQuadUV({ pos.x + startX, pos.y + startY }, { width, cursorHeight }, color, tex, { cursor.mUV, cursor.mSize }, typeFace->mTexture->size(), cursor.mFlipped);
            }
            ++index;

            unsigned char code = c > 0xFE ? '?' : c;
            const Render::Glyph &g = typeFace->mFonts.at(style)[code];

            float width = g.mSize.x * scale;
            float height = g.mSize.y * scale;

            float startX = cursorX + g.mBearing.x * scale;
            float startY = originY - g.mBearing.y * scale;

            if (shadowOffset.x != 0.0f || shadowOffset.y != 0.0f) {
                renderData.setSubLayer(baseLayer);
                ColorFrame shadowFrame = ColorRenderData { Color4 { 0.0f, 0.0f, 0.0f, 1.0f } }.frame(color.mPos, color.mSize);
                if (useSmallSize)
                    renderData.renderQuadUV({ pos.x + startX + shadowOffset.x * scale, pos.y + startY + shadowOffset.y * scale }, { width, height }, shadowFrame, tex, { g.mUV2, g.mSize2 }, typeFace->mTexture->size(), g.mFlipped2);
                else
                    renderData.renderQuadUV({ pos.x + startX + shadowOffset.x * scale, pos.y + startY + shadowOffset.y * scale }, { width, height }, shadowFrame, tex, { g.mUV, g.mSize }, typeFace->mTexture->size(), g.mFlipped);
            }

            renderData.setSubLayer(baseLayer + 1);
            if (useSmallSize)
                renderData.renderQuadUV({ pos.x + startX, pos.y + startY }, { width, height }, color, tex, { g.mUV2, g.mSize2 }, typeFace->mTexture->size(), g.mFlipped2);
            else
                renderData.renderQuadUV({ pos.x + startX, pos.y + startY }, { width, height }, color, tex, { g.mUV, g.mSize }, typeFace->mTexture->size(), g.mFlipped);

            cursorX += g.mAdvance / 64.0f * scale;
        }
        if (index == cursorIndex) {
            const Render::Glyph &cursor = typeFace->mFonts.at(style)['|'];

            float width = 3.0f * scale;

            float startX = cursorX - 1.5f * scale;
            float startY = originY - ref.mBearing.y * scale;

            renderData.setSubLayer(baseLayer + 2);
            if (useSmallSize)
                renderData.renderQuadUV({ pos.x + startX, pos.y + startY }, { width, cursorHeight }, color, tex, { cursor.mUV2, cursor.mSize2 }, typeFace->mTexture->size(), cursor.mFlipped2);
            else
                renderData.renderQuadUV({ pos.x + startX, pos.y + startY }, { width, cursorHeight }, color, tex, { cursor.mUV, cursor.mSize }, typeFace->mTexture->size(), cursor.mFlipped);
        }

        renderData.setSubLayer(baseLayer);
    }

    void TextRenderData::renderSelection(WidgetsRenderData &renderData, std::string_view text, Vector2 pos, Vector2 size, const Render::TypeFace *typeFace, Render::FontStyle style, float fontSize, Vector2 pivot, const Atlas2::Entry &entry, int selectionStart, int selectionEnd, ColorFrame color)
    {
        float scale = fontSize / Render::FontLoader::sFontSize;

        float minY = typeFace->mDescender / 64.0f * scale;
        float maxY = typeFace->mAscender / 64.0f * scale;
        float fullHeight = maxY - minY;
        float fullWidth = calculateWidth(text, typeFace, style, fontSize);

        float cursorX = (size.x - fullWidth) * pivot.x;
        float originY = (size.y - fullHeight) * pivot.y + maxY;

        float startX;
        float endX;

        size_t i = 0;
        for (char32_t c : StringUtil::parseUTF8(text)) {
            
            if (i == selectionStart)
                startX = cursorX;

            if (i == selectionEnd)
                endX = cursorX;

            ++i;

            unsigned char code = c > 0xFE ? '?' : c;
            const Render::Glyph &g = typeFace->mFonts.at(style)[code];

            cursorX += g.mAdvance / 64.0f * scale;
        }
        if (i == selectionStart)
            startX = cursorX;

        if (i == selectionEnd)
            endX = cursorX;


        const Render::Glyph &ref = typeFace->mFonts.at(style)['D'];

        float height = ref.mSize.y * scale;

        float startY = originY - ref.mBearing.y * scale;

        renderData.renderQuadUV({ pos.x + startX, pos.y + startY }, { endX - startX, height }, color, {}, entry.mArea, { 2048, 2048 }, entry.mFlipped);
    }

    float TextRenderData::calculateWidth(std::string_view text, const Render::TypeFace *typeFace, Render::FontStyle style, float fontSize)
    {
        float scale = fontSize / Render::FontLoader::sFontSize;

        float result = 0.0f;

        for (char32_t c : StringUtil::parseUTF8(text)) {
            unsigned char code = c > 0xFE ? '?' : c;
            const Render::Glyph &g = typeFace->mFonts.at(style)[code];

            result += g.mAdvance / 64.0f * scale;
        }

        return result;
    }

    float TextRenderData::calculateWidth(char c, const Render::TypeFace *typeFace, Render::FontStyle style, float fontSize)
    {
        float scale = fontSize / Render::FontLoader::sFontSize;

        const Render::Glyph &g = typeFace->mFonts.at(style)[c];

        return g.mAdvance / 64.0f * scale;
    }

    float TextRenderData::calculateLineHeight(const Render::TypeFace *typeFace, float fontSize)
    {
        float scale = fontSize / Render::FontLoader::sFontSize;

        float minY = typeFace->mDescender / 64.0f * scale;
        float maxY = typeFace->mAscender / 64.0f * scale;
        return maxY - minY;
    }

    Rect2 TextRenderData::calculateBoundingBox(const Line &line, size_t lineCount, size_t lineNr, Vector2 pos, Vector2 size, const Render::TypeFace *typeFace, float fontSize, Vector2 pivot)
    {
        float lineHeight = calculateLineHeight(typeFace, fontSize);
        float fullWidth = line.mWidth;

        float cursorX = (size.x - fullWidth) * pivot.x;
        float baseY = (size.y - lineHeight * lineCount) * pivot.y;

        return {
            pos + Vector2 { cursorX, baseY + lineHeight * lineNr },
            { fullWidth, lineHeight }
        };
    }

    Rect2 TextRenderData::calculateBoundingBox(std::string_view text, Vector2 pos, Vector2 size, const Render::TypeFace *typeFace, Render::FontStyle style, float fontSize, Vector2 pivot)
    {
        float fullWidth = calculateWidth(text, typeFace, style, fontSize);

        return calculateBoundingBox({ text.data(), text.data() + text.size(), fullWidth }, 1, 0, pos, size, typeFace, fontSize, pivot);
    }

}
}