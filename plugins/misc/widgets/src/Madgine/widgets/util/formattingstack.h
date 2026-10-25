#pragma once

#include "Meta/math/atlas2.h"

#include "Madgine/render/fonts/fontloader.h"

#include "colorrenderdata.h"

namespace Engine {
namespace Widgets {

    struct FormattingStack {

        FormattingStack(const Render::TypeFace *typeFace, Render::FontStyle style, float fontSize, ColorFrame color = {});
        ~FormattingStack();

        float renderGlyph(WidgetsRenderData &renderData, char32_t code, const Math::Vector2 &pos);
        void renderShadow(WidgetsRenderData &renderData, char32_t code, const Math::Vector2 &pos, const Math::Vector2 &shadowOffset);
        float advance(char32_t code);
        std::pair<float, float> place(float height);    

        std::pair<bool, std::string> parse(char32_t code);

        const Render::TypeFace *typeFace() const;

    private:
        Render::FontStyle mStyle;
        float mFontSize;
        ColorFrame mColor;
        const Render::TypeFace *mTypeFace;

        bool mEscaped = false;
        bool mFormatting = false;
        std::string mFormatToken;

        std::stack<std::pair<Math::Color4, Math::Color4>> mColorStack;
        std::stack<float> mSizeStack;
    };

}
}