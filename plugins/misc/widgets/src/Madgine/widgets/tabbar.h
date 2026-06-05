#pragma once

#include "util/colortintrenderdata.h"
#include "util/layouts/uniformlayoutrenderdata.h"
#include "util/scalableimagerenderdata.h"
#include "util/textrenderdata.h"
#include "widget.h"

namespace Engine {
namespace Widgets {
    struct MADGINE_WIDGETS_EXPORT TabBar : Widget<TabBar> {

        TabBar(WidgetManager &manager, WidgetBase *parent = nullptr);

        virtual ~TabBar() = default;

        Execution::SignalStub<void, uint32_t> &selectedTabChangedEvent();

        void render(WidgetsRenderData &renderData) override;

        void sizeChanged(const Math::Vector3 &pixelSize) override;

        uint32_t tabCount() const;
        void setTabCount(uint32_t count);

        uint32_t selectedTab() const;

        UniformLayoutRenderData mTabBarRenderData;
        TextRenderData mTextRenderData;
        ScalableImageRenderData mImageRenderData;
        ColorTintRenderData mColorTintRenderData;

        std::vector<std::string> mTabNames;

    protected:
        void injectPointerMove(const Platform::Input::PointerMoveEvent &arg) override;
        void injectPointerLeave(const Platform::Input::PointerMoveEvent &arg) override;

        void injectPointerClick(const PointerClickEvent &arg) override;

    private:
        Execution::Signal<void, uint32_t> mSelectedTabChanged;

        uint32_t mSelectedTab = 0;
        IndexType<uint32_t> mHoveredTab;
    };
}
}
