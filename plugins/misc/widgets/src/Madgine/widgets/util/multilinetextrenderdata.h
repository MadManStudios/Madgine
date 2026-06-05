#pragma once

#include "textrenderdata.h"

namespace Engine {
namespace Widgets {

    struct MultilineTextRenderData : TextRenderData {

        const std::vector<Line> &lines() const;

        void render(WidgetsRenderData &renderData, Math::Vector2 pos, Math::Vector3 size, int cursorIndex = -1) const;
        void renderSelection(WidgetsRenderData &renderData, Math::Vector2 pos, Math::Vector3 size, const Math::Atlas2::Entry &entry, int selectionStart, int selectionEnd, ColorFrame color);
        void updateText(std::string_view text, Math::Vector3 size, std::locale loc = {});
        float calculateTotalHeight(float z = 1.0f);

        static void renderText(WidgetsRenderData &renderData, const std::vector<Line> &lines, Math::Vector2 pos, Math::Vector2 size, const Render::TypeFace *typeFace, Render::FontStyle style, float fontSize, ColorFrame color, Math::Vector2 pivot, Math::Vector2 shadowOffset = { 0.0f, 0.0f }, int cursorIndex = -1);
        static void renderSelection(WidgetsRenderData &renderData, const std::vector<Line> &lines, Math::Vector2 pos, Math::Vector2 size, const Render::TypeFace *typeFace, Render::FontStyle style, float fontSize, Math::Vector2 pivot, const Math::Atlas2::Entry &entry, int selectionStart, int selectionEnd, ColorFrame color);
        static std::vector<Line> calculateLines(std::string_view text, Math::Vector2 size, const Render::TypeFace *typeFace, Render::FontStyle style, float fontSize, std::locale loc);
        static float calculateTotalHeight(size_t lineCount, const Render::TypeFace *typeFace, float fontSize);

    private:
        std::vector<Line> mLines;
    };

}
}