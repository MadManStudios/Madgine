#pragma once

#include "Meta/math/atlas2.h"
#include "Meta/math/color4.h"
#include "Meta/math/rect2.h"

#include "Madgine/render/fonts/fontloader.h"

#include "layouts/sizeconstraints.h"
#include "renderdata.h"
#include "texturesettings.h"
#include "vertex.h"
#include "colorrenderdata.h"

namespace Engine {
namespace Widgets {

    struct MADGINE_WIDGETS_EXPORT TextRenderData : RenderData {

        struct Line {
            const char *mBegin, *mEnd;
            float mWidth = 0;
        };

        std::string_view getFontName() const;
        void setFontName(std::string_view name);

        Render::FontLoader::Resource *getFont() const;
        void setFont(Render::FontLoader::Resource *font);

        bool available() const;
        void render(WidgetsRenderData &renderData, std::string_view text, Math::Vector2 pos, Math::Vector3 size, int cursorIndex = -1) const;
        void renderSelection(WidgetsRenderData &renderData, std::string_view text, Math::Vector2 pos, Math::Vector3 size, const Math::Atlas2::Entry &entry, int selectionStart, int selectionEnd, ColorFrame color);
        float calculateWidth(std::string_view text, float z = 1.0f);
        float calculateWidth(char c, float z = 1.0f);
        float calculateLineHeight(float z = 1.0f) const;
        Math::Rect2 calculateBoundingBox(const Line &line, size_t lineCount, size_t lineNr, Math::Vector2 pos, Math::Vector3 size);
        Math::Rect2 calculateBoundingBox(std::string_view text, Math::Vector2 pos, Math::Vector3 size);

        static void renderText(WidgetsRenderData &renderData, std::string_view text, Math::Vector2 pos, Math::Vector2 size, const Render::TypeFace *typeFace, Render::FontStyle style, float fontSize, ColorFrame color, Math::Vector2 pivot, Math::Vector2 shadowOffset = { 0.0f, 0.0f }, int cursorIndex = -1);
        static void renderLine(WidgetsRenderData &renderData, const Line &line, float originY, Math::Vector2 pos, Math::Vector2 size, const Render::TypeFace *typeFace, Render::FontStyle style, float fontSize, ColorFrame color, Math::Vector2 pivot, Math::Vector2 shadowOffset = { 0.0f, 0.0f }, int cursorIndex = -1);
        static void renderSelection(WidgetsRenderData &renderData, std::string_view text, Math::Vector2 pos, Math::Vector2 size, const Render::TypeFace *typeFace, Render::FontStyle style, float fontSize, Math::Vector2 pivot, const Math::Atlas2::Entry &entry, int selectionStart, int selectionEnd, ColorFrame color);
        static float calculateWidth(std::string_view text, const Render::TypeFace *typeFace, Render::FontStyle style, float fontSize);
        static float calculateWidth(char c, const Render::TypeFace *typeFace, Render::FontStyle style, float fontSize);
        static float calculateLineHeight(const Render::TypeFace *typeFace, float fontSize);
        static Math::Rect2 calculateBoundingBox(const Line &line, size_t lineCount, size_t lineNr, Math::Vector2 pos, Math::Vector2 size, const Render::TypeFace *typeFace, float fontSize, Math::Vector2 pivot);
        static Math::Rect2 calculateBoundingBox(std::string_view text, Math::Vector2 pos, Math::Vector2 size, const Render::TypeFace *typeFace, Render::FontStyle style, float fontSize, Math::Vector2 pivot);

        int mFontSize = 16;

        Render::FontStyle mStyle = Render::FontStyle::Default;

        Math::Vector2 mPivot = { 0.5f, 0.5f };

        Math::Vector2 mShadowOffset = { 0.0f, 0.0f };

        ColorRenderData mColor;

    protected:
        Render::FontLoader::Handle mFont;
        Resources::ImageLoader::Resource *mImage = nullptr;
    };

}
}