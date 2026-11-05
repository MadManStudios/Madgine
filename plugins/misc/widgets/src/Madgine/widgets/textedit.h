#pragma once

#include "util/multilinetextrenderdata.h"
#include "util/scalableimagerenderdata.h"
#include "widget.h"

#define STB_TEXTEDIT_CHARTYPE char
#define STB_TEXTEDIT_POSTIONTYPE size_t
#define STB_TEXTEDIT_UNDOSTATECOUNT 1
#define STB_TEXTEDIT_UNDOCHARCOUNT 1
#include "stb_textedit.h"

namespace Engine {
namespace Widgets {
    struct MADGINE_WIDGETS_EXPORT TextEdit : Widget<TextEdit> {
        TextEdit(WidgetManager &manager, WidgetBase *parent = nullptr);
        virtual ~TextEdit() = default;

        void setText(std::string_view text);
        std::string_view text() const;

        size_t size() const;
        char at(size_t i) const;
        void erase(size_t where, size_t n);
        bool insert(size_t where, std::string_view s);

        bool editable() const;

        void setEditable(bool b);

        void render(WidgetsRenderData &renderData) override;

        void sizeChanged(const Math::Vector3 &pixelSize) override;

        void onActivate(Serialize::CallbackTiming, bool b);

        void injectPointerClick(const PointerClickEvent &arg) override;
        void injectDragBegin(const DragBeginEvent &arg) override;
        void injectDragMove(const DragMoveEvent &arg) override;
        bool injectKeyPress(const Platform::Input::KeyPressEvent &arg) override;
        bool injectTextInput(const Platform::Input::TextInputEvent &arg) override;

        bool injectAxisEvent(const Platform::Input::AxisEvent &arg) override;

        void layoutRow(StbTexteditRow *row, size_t i);
        float calculateWidth(size_t i, size_t n);
        Math::Vector3 getAbsoluteTextSize();

        void setBorder(Math::Vector2 border);
        Math::Vector2 border() const;

        MultilineTextRenderData mTextRenderData;

        ScalableImageRenderData mImageRenderData;

        float mVerticalScroll = 0.0f;

    private:
        STB_TexteditState mState;
        std::string mText;
        bool mEditable = true;

        Math::Vector2 mBorder = Math::Vector2::ZERO;
    };
}
}
