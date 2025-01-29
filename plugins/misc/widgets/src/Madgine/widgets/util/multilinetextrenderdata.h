#pragma once

#include "textrenderdata.h"

namespace Engine {
namespace Widgets {

    struct MultilineTextRenderData : TextRenderData {

        const std::vector<Line> &lines() const;

        void render(WidgetsRenderData &renderData, Vector3 pos, Vector3 size, int cursorIndex = -1) const;
        void renderSelection(WidgetsRenderData &renderData, Vector3 pos, Vector3 size, const Atlas2::Entry &entry, int selectionStart, int selectionEnd, Color4 color);        
        void updateText(std::string_view text, Vector3 size, std::locale loc = {});        
        float calculateTotalHeight(float z = 1.0f);

        static void renderText(WidgetsRenderData &renderData, const std::vector<Line> &lines, Vector3 pos, Vector2 size, const Render::Font *font, float fontSize, Color4 color, Vector2 pivot, int cursorIndex = -1);
        static void renderSelection(WidgetsRenderData &renderData, const std::vector<Line> &lines, Vector3 pos, Vector2 size, const Render::Font *font, float fontSize, Vector2 pivot, const Atlas2::Entry &entry, int selectionStart, int selectionEnd, Color4 color);
        static std::vector<Line> calculateLines(std::string_view text, Vector2 size, const Render::Font *font, float fontSize, std::locale loc);
        static float calculateTotalHeight(size_t lineCount, const Render::Font *font, float fontSize);

    private:
        std::vector<Line> mLines;
    };

}
}