#include "../../widgetslib.h"

#include "multilinetextrenderdata.h"

#include "Meta/reflect/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

#include "../widgetmanager.h"
#include "formattingstack.h"
#include "widgetsrenderdata.h"

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

    void MultilineTextRenderData::render(WidgetsRenderData &renderData, Math::Vector2 pos, Math::Vector3 size, int cursorIndex) const
    {
        renderText(renderData, mLines, pos, size.xy(), mFont, mStyle, size.z * mFontSize, mColor.frame(pos, size.xy()), mPivot, mShadowOffset, cursorIndex);
    }

    void MultilineTextRenderData::renderSelection(WidgetsRenderData &renderData, Math::Vector2 pos, Math::Vector3 size, const Math::Atlas2::Entry &entry, int selectionStart, int selectionEnd, ColorFrame color)
    {
        renderSelection(renderData, mLines, pos, size.xy(), mFont, mStyle, size.z * mFontSize, mPivot, entry, selectionStart, selectionEnd, color);
    }

    void MultilineTextRenderData::updateText(WidgetManager &manager, std::string_view text, Math::Vector3 size, std::locale loc)
    {
        FormattingStack stack { mFont, mStyle, size.z * mFontSize };

        mLines = calculateLines(manager, text, size.xy(), stack, loc);
    }

    float MultilineTextRenderData::calculateTotalHeight(float z)
    {
        if (!mFont.available())
            return 0.0f;
        return calculateTotalHeight(mLines.size(), mFont, z * mFontSize);
    }

    void MultilineTextRenderData::renderText(WidgetsRenderData &renderData, const std::vector<Line> &lines, Math::Vector2 pos, Math::Vector2 size, const Render::TypeFace *typeFace, Render::FontStyle style, float fontSize, ColorFrame color, Math::Vector2 pivot, Math::Vector2 shadowOffset, int cursorIndex)
    {
        float scale = fontSize / Render::FontLoader::sFontSize;

        float minY = typeFace->mDescender / 64.0f * scale;
        float maxY = typeFace->mAscender / 64.0f * scale;
        float lineHeight = maxY - minY;

        float originY = std::max(size.y - lineHeight * lines.size(), 0.0f) * pivot.y + minY;

        FormattingStack stack { typeFace, style, fontSize, color };

        for (const Line &line : lines) {
            originY += lineHeight;

            renderLine(renderData, line, originY, pos, size, stack, pivot, shadowOffset, cursorIndex == -1 ? -1 : cursorIndex - (line.mBegin - lines.front().mBegin));
        }
    }

    void MultilineTextRenderData::renderSelection(WidgetsRenderData &renderData, const std::vector<Line> &lines, Math::Vector2 pos, Math::Vector2 size, const Render::TypeFace *typeFace, Render::FontStyle style, float fontSize, Math::Vector2 pivot, const Math::Atlas2::Entry &entry, int selectionStart, int selectionEnd, ColorFrame color)
    {
        if (selectionStart > selectionEnd)
            std::swap(selectionStart, selectionEnd);

        float scale;

        const Render::TypeFace::GlyphSet &set = typeFace->mFonts.at(style).set(fontSize, &scale);

        const Render::Glyph &ref = set['D'];
        float height = ref.mSize.y * scale;

        float minY = typeFace->mDescender / 64.0f * fontSize / Render::FontLoader::sFontSize;
        float maxY = typeFace->mAscender / 64.0f * fontSize / Render::FontLoader::sFontSize;
        float lineHeight = maxY - minY;

        float originY = std::max(size.y - lineHeight * lines.size(), 0.0f) * pivot.y + minY;

        const char *textBegin = lines.front().mBegin;
        const char *selectionStartPtr = textBegin + selectionStart;
        const char *selectionEndPtr = textBegin + selectionEnd;

        renderData.setSubLayer(1);

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

                    const Render::Glyph &g = set[static_cast<uint8_t>(*c)];

                    cursorX += g.mAdvance / 64.0f * scale;
                }
            }

            float startY = originY - ref.mBearing.y * scale;

            renderData.renderQuadUV(
                { pos.x + startX, pos.y + startY }, { endX - startX, height }, color, {}, entry.mArea, { 2048, 2048 }, entry.mFlipped);
        }
    }

    std::vector<TextRenderData::Line> MultilineTextRenderData::calculateLines(WidgetManager &manager, std::string_view text, Math::Vector2 size, FormattingStack &stack, std::locale loc)
    {
        if (!stack.typeFace())
            return {};

        const std::ctype<char> &ctype = std::use_facet<std::ctype<char>>(loc);

        std::vector<Line> lines;

        float currentLineWidth = 0.0f;
        const char *end = text.data() + text.size();
        const char *sol = text.data();
        const char *lastSpace = sol;
        float lastSpaceWidth = 0.0f;
        float lastSpaceGlyphWidth = 0.0f;
        for (const char *cursor = sol; cursor != end; ++cursor) {

            float width;

            auto [consumed, image] = stack.parse(static_cast<unsigned char>(*cursor));

            if (!image.empty()) {
                const Math::Atlas2::Entry *entry = manager.lookUpImage(image);
                if (!entry)
                    continue;
                width = stack.place(entry->mArea.mSize.y).second * entry->mArea.mSize.x;
            } else if (consumed) {
                continue;
            } else {
                width = stack.advance(static_cast<unsigned char>(*cursor));
            }

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

    float MultilineTextRenderData::calculateTotalHeight(size_t lineCount, const Render::TypeFace *typeFace, float fontSize)
    {
        return lineCount * calculateLineHeight(typeFace, fontSize);
    }

}
}