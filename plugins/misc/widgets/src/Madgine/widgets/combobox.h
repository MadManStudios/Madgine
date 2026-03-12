#pragma once

#include "util/colortintrenderdata.h"
#include "util/scalableimagerenderdata.h"
#include "util/textrenderdata.h"
#include "widget.h"

namespace Engine {
namespace Widgets {
    struct MADGINE_WIDGETS_EXPORT Combobox : Widget<Combobox> {

        Combobox(WidgetManager &manager, WidgetBase *parent = nullptr);

        virtual ~Combobox() = default;

        void render(WidgetsRenderData &renderData) override;

        Execution::SignalStub<void, size_t> &indexChangedEvent();

        void addItem(const std::string &text);
        void clear();

        void setCurrentIndex(size_t index);
        size_t currentIndex() const;

        void setEnabled(bool enabled);
        bool isEnabled() const;
        
        std::vector<std::string> mItems;

        TextRenderData mTextRenderData;
        ScalableImageRenderData mBackgroundRenderData;
        ScalableImageRenderData mButtonRenderData;
        ColorTintRenderData mColorTintRenderData;
        ColorRenderData mSelectionColorRenderData;

        float mSpacing = 2.0f;

    protected:
        bool containsPoint(const Vector2 &point, const Rect2i &screenSpace, float extend) const override;

        void injectPointerEnter(const Input::PointerMoveEvent &arg) override;
        void injectPointerLeave(const Input::PointerMoveEvent &arg) override;
        void injectPointerMove(const Input::PointerMoveEvent &arg) override;

        void injectPointerClick(const PointerClickEvent &arg) override;

        void onFocusLost() override;

    private:
        bool mHovered = false;
        bool mEnabled = true;
        bool mOpen = false;
        int mHoveredIndex = -1;
        size_t mSelectedIndex = 0;

        Execution::Signal<void, size_t> mIndexChanged;
    };
}
}
