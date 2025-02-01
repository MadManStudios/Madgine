#pragma once

#include "renderdata.h"

#include "Madgine/render/fonts/fontloader.h"

#include "Meta/math/atlas2.h"
#include "Meta/math/color4.h"
#include "Meta/math/rect2.h"

#include "layouts/sizeconstraints.h"
#include "texturesettings.h"
#include "vertex.h"

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
        void render(WidgetsRenderData &renderData, std::string_view text, Vector3 pos, Vector3 size, int cursorIndex = -1) const;
        void renderSelection(WidgetsRenderData &renderData, std::string_view text, Vector3 pos, Vector3 size, const Atlas2::Entry &entry, int selectionStart, int selectionEnd, Color4 color);
        float calculateWidth(std::string_view text, float z = 1.0f);
        float calculateWidth(char c, float z = 1.0f);
        float calculateLineHeight(float z = 1.0f);
        Rect2 calculateBoundingBox(const Line &line, size_t lineCount, size_t lineNr, Vector2 pos, Vector3 size);
        Rect2 calculateBoundingBox(std::string_view text, Vector2 pos, Vector3 size);

        static void renderText(WidgetsRenderData &renderData, std::string_view text, Vector3 pos, Vector2 size, const Render::Font *font, float fontSize, Color4 color, Vector2 pivot, int cursorIndex = -1);
        static void renderLine(WidgetsRenderData &renderData, const Line &line, float originY, Vector3 pos, Vector2 size, const Render::Font *font, float fontSize, Color4 color, Vector2 pivot, int cursorIndex = -1);
        static void renderSelection(WidgetsRenderData &renderData, std::string_view text, Vector3 pos, Vector2 size, const Render::Font *font, float fontSize, Vector2 pivot, const Atlas2::Entry &entry, int selectionStart, int selectionEnd, Color4 color);
        static float calculateWidth(std::string_view text, const Render::Font *font, float fontSize);
        static float calculateWidth(char c, const Render::Font *font, float fontSize);
        static float calculateLineHeight(const Render::Font *font, float fontSize);
        static Rect2 calculateBoundingBox(const Line &line, size_t lineCount, size_t lineNr, Vector2 pos, Vector2 size, const Render::Font *font, float fontSize, Vector2 pivot);
        static Rect2 calculateBoundingBox(std::string_view text, Vector2 pos, Vector2 size, const Render::Font *font, float fontSize, Vector2 pivot);

        int mFontSize = 16;

        Vector2 mPivot = { 0.5f, 0.5f };

        Color4 mColor = { 1, 1, 1, 1 };

    protected:
        Render::FontLoader::Handle mFont;
        Resources::ImageLoader::Resource *mImage = nullptr;
    };

}
}