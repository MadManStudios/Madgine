#pragma once

#include "util/scalableimagerenderdata.h"
#include "util/textrenderdata.h"
#include "widget.h"

#define STB_TEXTEDIT_CHARTYPE char
#define STB_TEXTEDIT_POSTIONTYPE size_t
#define STB_TEXTEDIT_UNDOSTATECOUNT 1
#define STB_TEXTEDIT_UNDCHARCOUNT 1
#include "stb_textedit.h"

namespace Engine {
namespace Widgets {
    struct MADGINE_WIDGETS_EXPORT Textbox : Widget<Textbox> {
        Textbox(WidgetManager &manager, WidgetBase *parent = nullptr);
        virtual ~Textbox() = default;

        bool editable() const;

        void setEditable(bool b);

        void render(WidgetsRenderData &renderData) override;

        void injectPointerClick(const PointerClickEvent &arg) override;
        void injectDragBegin(const DragBeginEvent &arg) override;
        void injectDragMove(const DragMoveEvent &arg) override;
        bool injectKeyPress(const Platform::Input::KeyPressEvent &arg) override;

        void layoutRow(StbTexteditRow *row, size_t i);
        float calculateWidth(size_t i, size_t n);

        TextRenderData mTextRenderData;
        std::string mText;

        ScalableImageRenderData mImageRenderData;

    private:
        STB_TexteditState mState;
    };
}
}
