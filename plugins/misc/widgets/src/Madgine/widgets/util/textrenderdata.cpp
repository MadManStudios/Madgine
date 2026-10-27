#include "../../widgetslib.h"

#include "textrenderdata.h"

#include "Madgine/render/texture.h"

#include "Meta/reflect/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

#include "../widgetmanager.h"
#include "formattingstack.h"
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

    Threading::TaskFuture<bool> TextRenderData::loadingState()
    {
        return mFont ? mFont.info()->loadingTask() : Threading::TaskFuture<bool> { false };
    }

    void TextRenderData::render(WidgetsRenderData &renderData, std::string_view text, Math::Vector2 pos, Math::Vector3 size, int cursorIndex) const
    {
        renderText(renderData, text, pos, size.xy(), mFont, mStyle, size.z * mFontSize, mColor.frame(pos, size.xy()), mPivot, mShadowOffset, cursorIndex);
    }

    void TextRenderData::renderSelection(WidgetsRenderData &renderData, std::string_view text, Math::Vector2 pos, Math::Vector3 size, const Math::Atlas2::Entry &entry, int selectionStart, int selectionEnd, ColorFrame color)
    {
        renderSelection(renderData, text, pos, size.xy(), mFont, mStyle, size.z * mFontSize, mPivot, entry, selectionStart, selectionEnd, color);
    }

    float TextRenderData::calculateWidth(WidgetManager &manager, std::string_view text, float z)
    {
        FormattingStack stack { mFont, mStyle, z * mFontSize };

        return calculateWidth(manager, text, stack);
    }

    float TextRenderData::calculateWidth(char c, float z)
    {
        FormattingStack stack { mFont, mStyle, z * mFontSize };

        return calculateWidth(c, stack);
    }

    float TextRenderData::calculateLineHeight(float z) const
    {
        return calculateLineHeight(mFont, z * mFontSize);
    }

    Math::Rect2 TextRenderData::calculateBoundingBox(const Line &line, size_t lineCount, size_t lineNr, Math::Vector2 pos, Math::Vector3 size)
    {
        return calculateBoundingBox(line, lineCount, lineNr, pos, size.xy(), mFont, size.z * mFontSize, mPivot);
    }

    Math::Rect2 TextRenderData::calculateBoundingBox(WidgetManager &manager, std::string_view text, Math::Vector2 pos, Math::Vector3 size)
    {
        return calculateBoundingBox(manager, text, pos, size.xy(), mFont, mStyle, size.z * mFontSize, mPivot);
    }

    void TextRenderData::renderText(WidgetsRenderData &renderData, std::string_view text, Math::Vector2 pos, Math::Vector2 size, const Render::TypeFace *typeFace, Render::FontStyle style, float fontSize, ColorFrame color, Math::Vector2 pivot, Math::Vector2 shadowOffset, int cursorIndex)
    {
        if (text.empty() && cursorIndex == -1)
            return;

        float scale = fontSize / Render::FontLoader::sFontSize;

        float minY = typeFace->mDescender / 64.0f * scale;
        float maxY = typeFace->mAscender / 64.0f * scale;
        float fullHeight = maxY - minY;
        FormattingStack stack { typeFace, style, fontSize, color };
        float fullWidth = calculateWidth(renderData.manager(), text, stack);

        float originY = (size.y - fullHeight) * pivot.y + maxY;

        FormattingStack stack2 { typeFace, style, fontSize, color };

        renderLine(renderData, { text.data(), text.data() + text.size(), fullWidth }, originY, pos, size, stack2, pivot, shadowOffset, cursorIndex);
    }

    void TextRenderData::renderLine(WidgetsRenderData &renderData, const Line &line, float originY, Math::Vector2 pos, Math::Vector2 size, FormattingStack &stack, Math::Vector2 pivot, Math::Vector2 shadowOffset, int cursorIndex)
    {
        float cursorX = (size.x - line.mWidth) * pivot.x;

        size_t baseLayer = renderData.subLayer();

        size_t index = 0;

        for (char32_t c : StringUtil::parseUTF8({ line.mBegin, line.mEnd })) {

            auto [consumed, image] = stack.parse(c);

            if (!image.empty()) {
                const Math::Atlas2::Entry *entry = renderData.manager().lookUpImage(image);
                if (entry) {

                    auto [baseOffset, scale] = stack.place(entry->mArea.mSize.y);
                    renderData.renderQuadUV({ pos.x + cursorX, pos.y + originY - baseOffset }, scale * Math::Vector2 { entry->mArea.mSize }, ColorRenderData { Math::Color4 { 1.0f, 1.0f, 1.0f, 1.0f } }.frame(Math::Vector2::ZERO, Math::Vector3::ZERO), {}, entry->mArea, { 2048, 2048 });
                    cursorX += scale * entry->mArea.mSize.x;
                }
            }
            if (consumed)
                continue;

            if (index == cursorIndex) {
                renderData.setSubLayer(baseLayer + 2);
                stack.renderGlyph(renderData, '|', { pos.x + cursorX, pos.y + originY });
            }
            ++index;

            unsigned char code = c > 0xFE ? '?' : c;

            if (shadowOffset.x != 0.0f || shadowOffset.y != 0.0f) {
                renderData.setSubLayer(baseLayer);
                stack.renderShadow(renderData, code, { pos.x + cursorX, pos.y + originY }, shadowOffset);                
            }

            renderData.setSubLayer(baseLayer + 1);
            cursorX += stack.renderGlyph(renderData, code, { pos.x + cursorX, pos.y + originY });
        }
        if (index == cursorIndex) {
            renderData.setSubLayer(baseLayer + 2);
            stack.renderGlyph(renderData, '|', { pos.x + cursorX, pos.y + originY });
        }

        renderData.setSubLayer(baseLayer);
    }

    void TextRenderData::renderSelection(WidgetsRenderData &renderData, std::string_view text, Math::Vector2 pos, Math::Vector2 size, const Render::TypeFace *typeFace, Render::FontStyle style, float fontSize, Math::Vector2 pivot, const Math::Atlas2::Entry &entry, int selectionStart, int selectionEnd, ColorFrame color)
    {
        float scale = fontSize / Render::FontLoader::sFontSize;

        float minY = typeFace->mDescender / 64.0f * scale;
        float maxY = typeFace->mAscender / 64.0f * scale;
        float fullHeight = maxY - minY;
        FormattingStack stack { typeFace, style, fontSize };
        float fullWidth = calculateWidth(renderData.manager(), text, stack);

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

    float TextRenderData::calculateWidth(WidgetManager &manager, std::string_view text, FormattingStack &stack)
    {
        float result = 0.0f;

        for (char32_t c : StringUtil::parseUTF8(text)) {
            auto [consumed, image] = stack.parse(c);
            if (!image.empty()) {
                const Math::Atlas2::Entry *entry = manager.lookUpImage(image);
                if (!entry)
                    continue;
                result += stack.place(entry->mArea.mSize.y).second * entry->mArea.mSize.x;
            } else if (!consumed)
                result += stack.advance(c);
        }

        return result;
    }

    float TextRenderData::calculateWidth(char c, FormattingStack &stack)
    {
        return stack.advance(c);
    }

    float TextRenderData::calculateLineHeight(const Render::TypeFace *typeFace, float fontSize)
    {
        float scale = fontSize / Render::FontLoader::sFontSize;

        float minY = typeFace->mDescender / 64.0f * scale;
        float maxY = typeFace->mAscender / 64.0f * scale;
        return maxY - minY;
    }

    Math::Rect2 TextRenderData::calculateBoundingBox(const Line &line, size_t lineCount, size_t lineNr, Math::Vector2 pos, Math::Vector2 size, const Render::TypeFace *typeFace, float fontSize, Math::Vector2 pivot)
    {
        float lineHeight = calculateLineHeight(typeFace, fontSize);
        float fullWidth = line.mWidth;

        float cursorX = (size.x - fullWidth) * pivot.x;
        float baseY = (size.y - lineHeight * lineCount) * pivot.y;

        return {
            pos + Math::Vector2 { cursorX, baseY + lineHeight * lineNr },
            { fullWidth, lineHeight }
        };
    }

    Math::Rect2 TextRenderData::calculateBoundingBox(WidgetManager &manager, std::string_view text, Math::Vector2 pos, Math::Vector2 size, const Render::TypeFace *typeFace, Render::FontStyle style, float fontSize, Math::Vector2 pivot)
    {
        FormattingStack stack { typeFace, style, fontSize };
        float fullWidth = calculateWidth(manager, text, stack);

        return calculateBoundingBox({ text.data(), text.data() + text.size(), fullWidth }, 1, 0, pos, size, typeFace, fontSize, pivot);
    }

}
}