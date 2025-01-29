#pragma once

#include "widget.h"

#include "util/colortintrenderdata.h"
#include "util/layouts/uniformlayoutrenderdata.h"
#include "util/scalableimagerenderdata.h"
#include "util/textrenderdata.h"

namespace Engine {
namespace Widgets {
    struct MADGINE_WIDGETS_EXPORT TabBar : Widget<TabBar> {

        TabBar(WidgetManager &manager, WidgetBase *parent = nullptr);

        virtual ~TabBar() = default;

        Execution::SignalStub<uint32_t> &selectedTabChangedEvent();

        void vertices(WidgetsRenderData &renderData, size_t layer) override;

        virtual WidgetClass getClass() const override;

        virtual void sizeChanged(const Vector3 &pixelSize) override;

        uint32_t tabCount() const;
        void setTabCount(uint32_t count);

        uint32_t selectedTab() const;

        UniformLayoutRenderData mTabBarRenderData;
        TextRenderData mTextRenderData;
        ScalableImageRenderData mImageRenderData;
        ColorTintRenderData mColorTintRenderData;

        std::vector<std::string> mTabNames;

    protected:
        void injectPointerMove(const Input::PointerEventArgs &arg) override;
        void injectPointerLeave(const Input::PointerEventArgs &arg) override;

        void injectPointerClick(const Input::PointerEventArgs &arg) override;      

    private:
        Execution::Signal<uint32_t> mSelectedTabChanged;

        uint32_t mSelectedTab = 0;
        IndexType<uint32_t> mHoveredTab;
    };
}
}

REGISTER_TYPE(Engine::Widgets::TabBar)
