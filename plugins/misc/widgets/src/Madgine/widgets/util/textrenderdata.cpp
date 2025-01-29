#include "../../widgetslib.h"

#include "textrenderdata.h"

#include "widgetsrenderdata.h"

#include "Meta/keyvalue/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

METATABLE_BEGIN_BASE(Engine::Widgets::TextRenderData, Engine::Widgets::RenderData)
MEMBER(mFontSize)
MEMBER(mPivot)
MEMBER(mColor)
PROPERTY(Font, getFont, setFont)
METATABLE_END(Engine::Widgets::TextRenderData)

SERIALIZETABLE_INHERIT_BEGIN(Engine::Widgets::TextRenderData, Engine::Widgets::RenderData)
FIELD(mFontSize)
FIELD(mPivot)
FIELD(mColor)
ENCAPSULATED_FIELD(mFont, getFontName, setFontName)
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

    void TextRenderData::render(WidgetsRenderData &renderData, std::string_view text, Vector3 pos, Vector3 size, int cursorIndex) const
    {
        renderText(renderData, text, pos, size.xy(), mFont, size.z * mFontSize, mColor, mPivot, cursorIndex);
    }

    void TextRenderData::renderSelection(WidgetsRenderData &renderData, std::string_view text, Vector3 pos, Vector3 size, const Atlas2::Entry &entry, int selectionStart, int selectionEnd, Color4 color)
    {
        renderSelection(renderData, text, pos, size.xy(), mFont, size.z * mFontSize, mPivot, entry, selectionStart, selectionEnd, color);
    }

    float TextRenderData::calculateWidth(std::string_view text, float z)
    {
        return calculateWidth(text, mFont, z * mFontSize);
    }

    float TextRenderData::calculateWidth(char c, float z)
    {
        return calculateWidth(c, mFont, z * mFontSize);
    }

    float TextRenderData::calculateLineHeight(float z)
    {
        return calculateLineHeight(mFont, z * mFontSize);
    }

    Rect2 TextRenderData::calculateBoundingBox(const Line &line, size_t lineCount, size_t lineNr, Vector2 pos, Vector3 size)
    {
        return calculateBoundingBox(line, lineCount, lineNr, pos, size.xy(), mFont, size.z * mFontSize, mPivot);
    }

    Rect2 TextRenderData::calculateBoundingBox(std::string_view text, Vector2 pos, Vector3 size)
    {
        return calculateBoundingBox(text, pos, size.xy(), mFont, size.z * mFontSize, mPivot);
    }

    void TextRenderData::renderText(WidgetsRenderData &renderData, std::string_view text, Vector3 pos, Vector2 size, const Render::Font *font, float fontSize, Color4 color, Vector2 pivot, int cursorIndex)
    {
        size_t textLen = text.size();

        if (textLen == 0 && cursorIndex == -1)
            return;

        float scale = fontSize / 64.0f;

        float minY = font->mDescender / 64.0f * scale;
        float maxY = font->mAscender / 64.0f * scale;
        float fullHeight = maxY - minY;
        float fullWidth = calculateWidth(text, font, fontSize);

        float originY = (size.y - fullHeight) * pivot.y + maxY;

        renderLine(renderData, { text.data(), text.data() + text.size(), fullWidth }, originY, pos, size, font, fontSize, color, pivot, cursorIndex);
    }

    void TextRenderData::renderLine(WidgetsRenderData &renderData, const Line &line, float originY, Vector3 pos, Vector2 size, const Render::Font *font, float fontSize, Color4 color, Vector2 pivot, int cursorIndex)
    {
        float scale = fontSize / 64.0f;

        float cursorX = (size.x - line.mWidth) * pivot.x;

        bool useSmallSize = fontSize < 32;

        TextureSettings tex { font->mTexture->resource(), useSmallSize ? 0 : TextureFlag_IsDistanceField };

        const Render::Glyph &ref = font->mGlyphs['D'];

        float cursorHeight = ref.mSize.y * scale;

        for (const char *c = line.mBegin; c <= line.mEnd; ++c) {

            if (c - line.mBegin == cursorIndex) {
                const Render::Glyph &cursor = font->mGlyphs['|'];

                float width = 3.0f * scale;

                float startX = cursorX - 1.5f * scale;
                float startY = originY - ref.mBearing.y * scale;

                if (useSmallSize)
                    renderData.renderQuadUV({ pos.x + startX, pos.y + startY, pos.z + 0.6f }, { width, cursorHeight }, color, tex, { cursor.mUV2, cursor.mSize2 }, font->mTexture->size(), cursor.mFlipped2);
                else
                    renderData.renderQuadUV({ pos.x + startX, pos.y + startY, pos.z + 0.6f }, { width, cursorHeight }, color, tex, { cursor.mUV, cursor.mSize }, font->mTexture->size(), cursor.mFlipped);
            }

            if (c == line.mEnd)
                break;

            const Render::Glyph &g = font->mGlyphs[static_cast<unsigned char>(*c)];

            float width = g.mSize.x * scale;
            float height = g.mSize.y * scale;

            float startX = cursorX + g.mBearing.x * scale;
            float startY = originY - g.mBearing.y * scale;

            if (useSmallSize)
                renderData.renderQuadUV({ pos.x + startX, pos.y + startY, pos.z + 0.5f }, { width, height }, color, tex, { g.mUV2, g.mSize2 }, font->mTexture->size(), g.mFlipped2);
            else
                renderData.renderQuadUV({ pos.x + startX, pos.y + startY, pos.z + 0.5f }, { width, height }, color, tex, { g.mUV, g.mSize }, font->mTexture->size(), g.mFlipped);

            cursorX += g.mAdvance / 64.0f * scale;
        }
    }

    void TextRenderData::renderSelection(WidgetsRenderData &renderData, std::string_view text, Vector3 pos, Vector2 size, const Render::Font *font, float fontSize, Vector2 pivot, const Atlas2::Entry &entry, int selectionStart, int selectionEnd, Color4 color)
    {
        size_t textLen = text.size();

        float scale = fontSize / 64.0f;

        float minY = font->mDescender / 64.0f * scale;
        float maxY = font->mAscender / 64.0f * scale;
        float fullHeight = maxY - minY;
        float fullWidth = calculateWidth(text, font, fontSize);

        float cursorX = (size.x - fullWidth) * pivot.x;
        float originY = (size.y - fullHeight) * pivot.y + maxY;

        float startX;
        float endX;

        for (size_t i = 0; i <= textLen; ++i) {
            if (i == selectionStart)
                startX = cursorX;

            if (i == selectionEnd)
                endX = cursorX;

            if (i == textLen)
                break;

            const Render::Glyph &g = font->mGlyphs[text[i]];

            cursorX += g.mAdvance / 64.0f * scale;
        }

        const Render::Glyph &ref = font->mGlyphs['D'];

        float height = ref.mSize.y * scale;

        float startY = originY - ref.mBearing.y * scale;

        renderData.renderQuadUV({ pos.x + startX, pos.y + startY, pos.z + 0.4f }, { endX - startX, height }, color, {}, entry.mArea, { 2048, 2048 }, entry.mFlipped);
    }

    float TextRenderData::calculateWidth(std::string_view text, const Render::Font *font, float fontSize)
    {
        float scale = fontSize / 64.0f;

        float result = 0.0f;

        for (char c : text) {
            const Render::Glyph &g = font->mGlyphs[static_cast<uint8_t>(c)];

            result += g.mAdvance / 64.0f * scale;
        }

        return result;
    }

    float TextRenderData::calculateWidth(char c, const Render::Font *font, float fontSize)
    {
        float scale = fontSize / 64.0f;

        const Render::Glyph &g = font->mGlyphs[c];

        return g.mAdvance / 64.0f * scale;
    }

    float TextRenderData::calculateLineHeight(const Render::Font *font, float fontSize)
    {
        float scale = fontSize / 64.0f;

        float minY = font->mDescender / 64.0f * scale;
        float maxY = font->mAscender / 64.0f * scale;
        return maxY - minY;
    }

    Rect2 TextRenderData::calculateBoundingBox(const Line &line, size_t lineCount, size_t lineNr, Vector2 pos, Vector2 size, const Render::Font *font, float fontSize, Vector2 pivot)
    {
        float lineHeight = calculateLineHeight(font, fontSize);
        float fullWidth = line.mWidth;

        float cursorX = (size.x - fullWidth) * pivot.x;
        float baseY = (size.y - lineHeight * lineCount) * pivot.y;

        return {
            pos + Vector2 { cursorX, baseY + lineHeight * lineNr },
            { fullWidth, lineHeight }
        };
    }

    Rect2 TextRenderData::calculateBoundingBox(std::string_view text, Vector2 pos, Vector2 size, const Render::Font *font, float fontSize, Vector2 pivot)
    {
        float fullWidth = calculateWidth(text, font, fontSize);

        return calculateBoundingBox({ text.data(), text.data() + text.size(), fullWidth }, 1, 0, pos, size, font, fontSize, pivot);
    }

}
}