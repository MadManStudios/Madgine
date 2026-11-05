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

    void MultilineTextRenderData::render(WidgetsRenderData &renderData, std::string_view text, Math::Vector2 pos, Math::Vector3 size, int cursorIndex) const
    {
        renderText(renderData, text, mLines, pos, size.xy(), mFont, mStyle, size.z * mFontSize, mColor.frame(pos, size.xy()), mPivot, mShadowOffset, cursorIndex, mParseFormatting);
    }

    void MultilineTextRenderData::renderSelection(WidgetsRenderData &renderData, std::string_view text, Math::Vector2 pos, Math::Vector3 size, const Math::Atlas2::Entry &entry, int selectionStart, int selectionEnd, ColorFrame color)
    {
        renderSelection(renderData, text, mLines, pos, size.xy(), mFont, mStyle, size.z * mFontSize, mPivot, entry, selectionStart, selectionEnd, color, mParseFormatting);
    }

    void MultilineTextRenderData::updateText(WidgetManager &manager, std::string_view text, Math::Vector3 size, std::locale loc)
    {
        FormattingStack stack { mFont, mStyle, size.z * mFontSize, mParseFormatting };

        mLines = calculateLines(manager, text, size.xy(), stack, loc);
    }

    float MultilineTextRenderData::calculateTotalHeight(float z)
    {
        if (!mFont.available())
            return 0.0f;
        return calculateTotalHeight(mLines.size(), mFont, z * mFontSize);
    }

    void MultilineTextRenderData::renderText(WidgetsRenderData &renderData, std::string_view text, const std::vector<Line> &lines, Math::Vector2 pos, Math::Vector2 size, const Render::TypeFace *typeFace, Render::FontStyle style, float fontSize, ColorFrame color, Math::Vector2 pivot, Math::Vector2 shadowOffset, int cursorIndex, bool parseFormatting)
    {
        float scale = fontSize / Render::FontLoader::sFontSize;

        float minY = typeFace->mDescender / 64.0f * scale;
        float maxY = typeFace->mAscender / 64.0f * scale;
        float lineHeight = maxY - minY;

        float originY = std::max(size.y - lineHeight * lines.size(), 0.0f) * pivot.y + minY;

        FormattingStack stack { typeFace, style, fontSize, parseFormatting, color };

        for (const Line &line : lines) {
            originY += lineHeight;

            renderLine(renderData, text, line, originY, pos, size, stack, pivot, shadowOffset, cursorIndex);
        }
    }

    void MultilineTextRenderData::renderSelection(WidgetsRenderData &renderData, std::string_view text, const std::vector<Line> &lines, Math::Vector2 pos, Math::Vector2 size, const Render::TypeFace *typeFace, Render::FontStyle style, float fontSize, Math::Vector2 pivot, const Math::Atlas2::Entry &entry, int selectionStart, int selectionEnd, ColorFrame color, bool parseFormatting)
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
        FormattingStack stack { typeFace, style, fontSize, parseFormatting };

        float originY = std::max(size.y - lineHeight * lines.size(), 0.0f) * pivot.y + minY;

        renderData.setSubLayer(1);

        for (const Line &line : lines) {
            size_t i = line.mBegin;

            float cursorX = (size.x - line.mWidth) * pivot.x;

            originY += lineHeight;

            if (i >= selectionEnd || i < selectionStart)
                continue;

            float startX = cursorX;
            float endX = cursorX + line.mWidth;

            for (char32_t c : StringUtil::parseUTF8(text) | std::views::drop(line.mBegin)) {
                if (i == selectionStart)
                    startX = cursorX;

                if (i == selectionEnd)
                    endX = cursorX;

                ++i;

                cursorX += stack.advance(c);

                if (i >= line.mEnd)
                    break;
            }
            if (i == selectionStart)
                startX = cursorX;

            if (i == selectionEnd)
                endX = cursorX;

            float startY = originY - ref.mBearing.y * scale;

            renderData.renderQuadUV({ pos.x + startX, pos.y + startY }, { endX - startX, height }, color, {}, entry.mArea, { 2048, 2048 }, entry.mFlipped);
        }
    }

    std::vector<TextRenderData::Line> MultilineTextRenderData::calculateLines(WidgetManager &manager, std::string_view text, Math::Vector2 size, FormattingStack &stack, std::locale loc)
    {
        if (!stack.typeFace())
            return {};

        const std::ctype<char> &ctype = std::use_facet<std::ctype<char>>(loc);

        std::vector<Line> lines;

        float currentLineWidth = 0.0f;

        size_t i = 0;

        size_t sol = 0;
        size_t lastSpace = 0;
        float lastSpaceWidth = 0.0f;
        float lastSpaceGlyphWidth = 0.0f;
        bool lastWasCarriageReturn = false;
        for (char32_t code : StringUtil::parseUTF8(text)) {

            float width;

            auto [consumed, image] = stack.parse(code);

            if (!image.empty()) {
                const Math::Atlas2::Entry *entry = manager.lookUpImage(image);
                if (!entry)
                    continue;
                width = stack.place(entry->mArea.mSize.y).second * entry->mArea.mSize.x;
            } else if (consumed) {
                continue;
            } else {
                width = stack.advance(code);
            }

            if (ctype.is(std::ctype_base::space, code)) {
                lastSpace = i;
                lastSpaceWidth = currentLineWidth;
                lastSpaceGlyphWidth = width;
            }

            currentLineWidth += width;

            if (code == '\n' || code == '\r' || (currentLineWidth > size.x && lastSpace != sol)) {
                if (code != '\n' && !lastWasCarriageReturn) {
                    lines.push_back({ sol, lastSpace, lastSpaceWidth });
                    ++lastSpace;
                    sol = lastSpace;
                    currentLineWidth -= lastSpaceWidth;
                    currentLineWidth -= lastSpaceGlyphWidth;
                }
            }
            lastWasCarriageReturn = code == '\r';
            ++i;
        }
        lines.push_back({ sol, i, currentLineWidth });

        return lines;
    }

    float MultilineTextRenderData::calculateTotalHeight(size_t lineCount, const Render::TypeFace *typeFace, float fontSize)
    {
        return lineCount * calculateLineHeight(typeFace, fontSize);
    }

}
}