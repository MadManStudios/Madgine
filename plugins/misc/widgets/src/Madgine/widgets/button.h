#pragma once

#include "widget.h"

#include "Generic/execution/signal.h"

#include "util/textrenderdata.h"
#include "util/scalableimagerenderdata.h"
#include "util/colortintrenderdata.h"

namespace Engine {
namespace Widgets {
    struct MADGINE_WIDGETS_EXPORT Button : Widget<Button> {

        SERIALIZABLEUNIT(Button)
        
        Button(WidgetManager &manager, WidgetBase *parent = nullptr);

        virtual ~Button() = default;

        Execution::SignalStub<> &clickEvent();

        void setEnabled(bool enabled);
        bool isEnabled() const;

        void render(WidgetsRenderData &renderData) override;

        const char *getClass() const override;

        TextRenderData mTextRenderData;
        ScalableImageRenderData mImageRenderData;     
        ColorTintRenderData mColorTintRenderData;

        std::string mText;

    protected:
        void injectPointerEnter(const Input::PointerMoveEvent &arg) override;
        void injectPointerLeave(const Input::PointerMoveEvent &arg) override;

        void injectPointerClick(const PointerClickEvent &arg) override;        

        void emitClicked();

    private:
        Execution::Signal<> mClicked;

        bool mHovered = false;

        bool mEnabled = true;
    };

}
}
