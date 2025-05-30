#include "../widgetslib.h"

#include "tabbar.h"

#include "Meta/keyvalue/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

#include "widgetmanager.h"

METATABLE_BEGIN_BASE(Engine::Widgets::TabBar, Engine::Widgets::WidgetBase)
MEMBER(mTabNames)
PROPERTY(TabCount, tabCount, setTabCount)
NAMED_MEMBER(TextData, mTextRenderData)
NAMED_MEMBER(ImageData, mImageRenderData)
NAMED_MEMBER(ColorTint, mColorTintRenderData)
METATABLE_END(Engine::Widgets::TabBar)

SERIALIZETABLE_INHERIT_BEGIN(Engine::Widgets::TabBar, Engine::Widgets::WidgetBase)
FIELD(mTabNames)
FIELD(mTextRenderData)
FIELD(mImageRenderData)
FIELD(mColorTintRenderData)
SERIALIZETABLE_END(Engine::Widgets::TabBar)

namespace Engine {
namespace Widgets {

    TabBar::TabBar(WidgetManager &manager, WidgetBase *parent)
        : Widget(manager, parent, { .acceptsPointerEvents = true })
    {
    }

    Execution::SignalStub<uint32_t> &TabBar::selectedTabChangedEvent()
    {
        return mSelectedTabChanged;
    }

    void TabBar::render(WidgetsRenderData &renderData)
    {
        if (mTextRenderData.available()) {

            const Atlas2::Entry *entry = manager().lookUpImage(mImageRenderData.image());
            if (entry) {

                Vector2 pos = getAbsolutePosition();
                Vector3 size = getAbsoluteSize();

                for (size_t tabIndex = 0; tabIndex < tabCount(); ++tabIndex) {

                    auto [xPos, xSize] = mTabBarRenderData.getElementDimensions(tabIndex);

                    Vector2 tabPos {
                        pos.x + xPos,
                        pos.y
                    };
                    Vector3 tabSize {
                        xSize,
                        size.y,
                        size.z
                    };

                    Color4 color = mColorTintRenderData.mNormalColor;
                    if (mHoveredTab == tabIndex)
                        color = mColorTintRenderData.mHighlightedColor;
                    else if (mSelectedTab == tabIndex)
                        color = mColorTintRenderData.mPressedColor;

                    renderData.setSubLayer(0);
                    mImageRenderData.renderImage(renderData, tabPos, tabSize.xy(), *entry, color);
                    renderData.setSubLayer(1);
                    mTextRenderData.render(renderData, mTabNames[tabIndex], tabPos, tabSize);
                }
            }
        }

        WidgetBase::render(renderData);
    }

    std::string TabBar::getClass() const
    {
        return "TabBar";
    }

    uint32_t TabBar::tabCount() const
    {
        return mTabNames.size();
    }

    void TabBar::setTabCount(uint32_t count)
    {
        mTabNames.resize(count);
        applyGeometry();
    }

    uint32_t TabBar::selectedTab() const
    {
        return mSelectedTab;
    }

    void TabBar::sizeChanged(const Vector3 &pixelSize)
    {
        mTabBarRenderData.update(tabCount(), { 0.0f, 1.0f, 100000.0f }, pixelSize.x);
    }

    void TabBar::injectPointerMove(const Input::PointerEventArgs &arg)
    {
        float x = arg.windowPosition.x;
        mHoveredTab = mTabBarRenderData.elementIndex(x);
        WidgetBase::injectPointerMove(arg);
    }

    void TabBar::injectPointerLeave(const Input::PointerEventArgs &arg)
    {
        mHoveredTab.reset();
        WidgetBase::injectPointerLeave(arg);
    }

    void TabBar::injectPointerClick(const Input::PointerEventArgs &arg)
    {
        assert(mHoveredTab);
        if (mSelectedTab != mHoveredTab) {
            mSelectedTab = mHoveredTab;
            mSelectedTabChanged.emit(mSelectedTab);
        }
        WidgetBase::injectPointerClick(arg);
    }

}
}
