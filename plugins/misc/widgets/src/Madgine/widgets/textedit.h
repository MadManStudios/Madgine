#pragma once

#include "widget.h"

#include "util/multilinetextrenderdata.h"
#include "util/scalableimagerenderdata.h"

#define STB_TEXTEDIT_CHARTYPE char
#define STB_TEXTEDIT_POSTIONTYPE size_t
#define STB_TEXTEDIT_UNDOSTATECOUNT 1
#define STB_TEXTEDIT_UNDCHARCOUNT 1
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

        void vertices(WidgetsRenderData &renderData, size_t layer) override;

        virtual void sizeChanged(const Vector3 &pixelSize) override;

        virtual WidgetClass getClass() const override;

        void onActivate(bool b);

        virtual void injectPointerClick(const Input::PointerEventArgs &arg) override;
        virtual void injectDragBegin(const Input::PointerEventArgs &arg) override;
        virtual void injectDragMove(const Input::PointerEventArgs &arg) override;
        virtual bool injectKeyPress(const Input::KeyEventArgs &arg) override;

        virtual bool injectAxisEvent(const Input::AxisEventArgs &arg) override;

        void layoutRow(StbTexteditRow *row, size_t i);
        float calculateWidth(size_t i, size_t n);
        Vector3 getAbsoluteTextSize();

        void setBorder(Vector2 border);
        Vector2 border() const;

        MultilineTextRenderData mTextRenderData;

        ScalableImageRenderData mImageRenderData;

        float mVerticalScroll = 0.0f;

    private:
        STB_TexteditState mState;
        std::string mText;
        bool mEditable = true;

        Vector2 mBorder = Vector2::ZERO;        
    };
}
}
