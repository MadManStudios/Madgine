#pragma once

#include "Generic/execution/signal.h"

#include "Madgine/behavior/bindable.h"

#include "util/colortintrenderdata.h"
#include "util/scalableimagerenderdata.h"
#include "util/textrenderdata.h"
#include "widget.h"

namespace Engine {
namespace Widgets {
    struct MADGINE_WIDGETS_EXPORT Button : Widget<Button> {

        SERIALIZABLEUNIT(Button)

        Button(WidgetManager &manager, WidgetBase *parent = nullptr);

        virtual ~Button() = default;

        Execution::SignalStub<void> &clickEvent();

        void setEnabled(bool enabled);
        bool isEnabled() const;

        void render(WidgetsRenderData &renderData) override;

        TextRenderData mTextRenderData;
        ScalableImageRenderData mImageRenderData;
        ColorTintRenderData mColorTintRenderData;

        Behavior::Bindable<std::string> mText;

    protected:
        void injectPointerEnter(const Input::PointerMoveEvent &arg) override;
        void injectPointerLeave(const Input::PointerMoveEvent &arg) override;

        void injectPointerClick(const PointerClickEvent &arg) override;

        void emitClicked();

    private:
        Execution::Signal<void> mClicked;

        bool mHovered = false;

        bool mEnabled = true;
    };

}
}
