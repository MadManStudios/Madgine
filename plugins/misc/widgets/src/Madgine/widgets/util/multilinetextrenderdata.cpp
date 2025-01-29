#include "../../widgetslib.h"

#include "multilinetextrenderdata.h"

#include "widgetsrenderdata.h"

#include "Meta/keyvalue/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

METATABLE_BEGIN_BASE(Engine::Widgets::MultilineTextRenderData, Engine::Widgets::TextRenderData)
METATABLE_END(Engine::Widgets::MultilineTextRenderData)

SERIALIZETABLE_INHERIT_BEGIN(Engine::Widgets::MultilineTextRenderData, Engine::Widgets::TextRenderData)
SERIALIZETABLE_END(Engine::Widgets::MultilineTextRenderData)

namespace Engine {
namespace Widgets {

    const std::vector<TextRenderData::Line> &MultilineTextRenderData::lines() const
    {
        return mLines;
    }

    void MultilineTextRenderData::render(WidgetsRenderData &renderData, Vector3 pos, Vector3 size, int cursorIndex) const
    {
        renderText(renderData, mLines, pos, size.xy(), mFont, size.z * mFontSize, mColor, mPivot, cursorIndex);
    }

    void MultilineTextRenderData::renderSelection(WidgetsRenderData &renderData, Vector3 pos, Vector3 size, const Atlas2::Entry &entry, int selectionStart, int selectionEnd, Color4 color)
    {
        renderSelection(renderData, mLines, pos, size.xy(), mFont, size.z * mFontSize, mPivot, entry, selectionStart, selectionEnd, color);
    }

    void MultilineTextRenderData::updateText(std::string_view text, Vector3 size, std::locale loc)
    {
        mLines = calculateLines(text, size.xy(), mFont, size.z * mFontSize, loc);
    }

    float MultilineTextRenderData::calculateTotalHeight(float z)
    {
        return calculateTotalHeight(mLines.size(), mFont, z * mFontSize);
    }

    void MultilineTextRenderData::renderText(WidgetsRenderData &renderData, const std::vector<Line> &lines, Vector3 pos, Vector2 size, const Render::Font *font, float fontSize, Color4 color, Vector2 pivot, int cursorIndex)
    {
        float scale = fontSize / 64.0f;

        float minY = font->mDescender / 64.0f * scale;
        float maxY = font->mAscender / 64.0f * scale;
        float lineHeight = maxY - minY;

        float originY = std::max(size.y - lineHeight * lines.size(), 0.0f) * pivot.y + minY;

        for (const Line &line : lines) {
            originY += lineHeight;

            renderLine(renderData, line, originY, pos, size, font, fontSize, color, pivot, cursorIndex == -1 ? -1 : cursorIndex - (line.mBegin - lines.front().mBegin));
        }
    }

    void MultilineTextRenderData::renderSelection(WidgetsRenderData &renderData, const std::vector<Line> &lines, Vector3 pos, Vector2 size, const Render::Font *font, float fontSize, Vector2 pivot, const Atlas2::Entry &entry, int selectionStart, int selectionEnd, Color4 color)
    {
        if (selectionStart > selectionEnd)
            std::swap(selectionStart, selectionEnd);

        float scale = fontSize / 64.0f;

        float minY = font->mDescender / 64.0f * scale;
        float maxY = font->mAscender / 64.0f * scale;
        float lineHeight = maxY - minY;

        const Render::Glyph &ref = font->mGlyphs['D'];
        float height = ref.mSize.y * scale;

        float originY = std::max(size.y - lineHeight * lines.size(), 0.0f) * pivot.y + minY;

        const char *textBegin = lines.front().mBegin;
        const char *selectionStartPtr = textBegin + selectionStart;
        const char *selectionEndPtr = textBegin + selectionEnd;

        for (const Line &line : lines) {
            float cursorX = (size.x - line.mWidth) * pivot.x;

            originY += lineHeight;

            if (line.mBegin >= selectionEndPtr || line.mEnd < selectionStartPtr)
                continue;

            float startX = cursorX;
            float endX = cursorX + line.mWidth;

            bool startsInLine = line.mBegin <= selectionStartPtr && selectionStartPtr <= line.mEnd;
            bool endsInLine = line.mBegin <= selectionEndPtr && selectionEndPtr <= line.mEnd;

            if (startsInLine || endsInLine) {
                for (const char *c = line.mBegin; c <= line.mEnd; ++c) {
                    if (c == selectionStartPtr)
                        startX = cursorX;

                    if (c == selectionEndPtr)
                        endX = cursorX;

                    if (c == line.mEnd)
                        break;

                    const Render::Glyph &g = font->mGlyphs[static_cast<uint8_t>(*c)];

                    cursorX += g.mAdvance / 64.0f * scale;
                }
            }

            float startY = originY - ref.mBearing.y * scale;

            renderData.renderQuadUV(
                { pos.x + startX, pos.y + startY, pos.z + 0.4f }, { endX - startX, height }, color, {}, entry.mArea, { 2048, 2048 }, entry.mFlipped);
        }
    }

    std::vector<TextRenderData::Line> MultilineTextRenderData::calculateLines(std::string_view text, Vector2 size, const Render::Font *font, float fontSize, std::locale loc)
    {
        if (!font)
            return {};

        const std::ctype<char> &ctype = std::use_facet<std::ctype<char>>(loc);

        float scale = fontSize / 64.0f;

        std::vector<Line> lines;

        const Render::Glyph &space = font->mGlyphs[' '];

        float currentLineWidth = 0.0f;
        const char *end = text.data() + text.size();
        const char *sol = text.data();
        const char *lastSpace = sol;
        float lastSpaceWidth = 0.0f;
        float lastSpaceGlyphWidth = 0.0f;
        for (const char *cursor = sol; cursor != end; ++cursor) {
            const Render::Glyph &g = font->mGlyphs[static_cast<unsigned char>(*cursor)];

            float width = g.mAdvance / 64.0f * scale;

            if (ctype.is(std::ctype_base::space, *cursor)) {
                lastSpace = cursor;
                lastSpaceWidth = currentLineWidth;
                lastSpaceGlyphWidth = width;
            }

            currentLineWidth += width;

            if (*cursor == '\n' || (currentLineWidth > size.x && lastSpace != sol)) {
                lines.push_back({ sol, lastSpace, lastSpaceWidth });
                ++lastSpace;
                sol = lastSpace;
                currentLineWidth -= lastSpaceWidth;
                currentLineWidth -= lastSpaceGlyphWidth;
            }
        }
        lines.push_back({ sol, end, currentLineWidth });

        return lines;
    }

    float MultilineTextRenderData::calculateTotalHeight(size_t lineCount, const Render::Font *font, float fontSize)
    {
        return lineCount * calculateLineHeight(font, fontSize);
    }

}
}