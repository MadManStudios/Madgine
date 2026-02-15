#include "../widgetslib.h"

#include "combobox.h"

#include "Meta/keyvalue/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

#include "widgetmanager.h"

NAMED_UNIQUECOMPONENT(Combobox, Engine::Widgets::Combobox);

METATABLE_BEGIN_BASE(Engine::Widgets::Combobox, Engine::Widgets::WidgetBase)
    MEMBER(mItems)
    NAMED_MEMBER(TextData, mTextRenderData)
    NAMED_MEMBER(BackgroundData, mBackgroundRenderData)
    NAMED_MEMBER(ButtonData, mButtonRenderData)
    NAMED_MEMBER(ColorTint, mColorTintRenderData)
    NAMED_MEMBER(SelectionColor, mSelectionColorRenderData)
    MEMBER(mSpacing)
METATABLE_END(Engine::Widgets::Combobox)

SERIALIZETABLE_INHERIT_BEGIN(Engine::Widgets::Combobox, Engine::Widgets::WidgetBase)
    FIELD(mItems)
    FIELD(mTextRenderData)
    FIELD(mBackgroundRenderData)
    FIELD(mButtonRenderData)
    FIELD(mColorTintRenderData)
    FIELD(mSelectionColorRenderData)
    FIELD(mSpacing)
SERIALIZETABLE_END(Engine::Widgets::Combobox)

namespace Engine {
namespace Widgets {

    Combobox::Combobox(WidgetManager &manager, WidgetBase *parent)
        : Widget(manager, parent, { .acceptsPointerEvents = true })
    {
    }

    void Combobox::render(WidgetsRenderData &renderData)
    {
        Vector2 pos = getAbsolutePosition();
        Vector3 size = getAbsoluteSize();

        const ColorRenderData &color = mEnabled ? ((mHovered || mOpen) ? mColorTintRenderData.mHighlightedColor : mColorTintRenderData.mNormalColor)
                                                : mColorTintRenderData.mDisabledColor;

        const Atlas2::Entry *background = manager().lookUpImage(mBackgroundRenderData.image());

        Vector3 currentSize { size.x - size.y, size.y, size.z };
        if (background) {
            renderData.setSubLayer(0);
            mBackgroundRenderData.renderImage(renderData, pos, currentSize, *background);
        }

        if (mTextRenderData.available() && mSelectedIndex >= 0 && mSelectedIndex < mItems.size()) {
            renderData.setSubLayer(1);
            mTextRenderData.render(renderData, mItems[mSelectedIndex], pos, currentSize);
        }

        const Atlas2::Entry *button = manager().lookUpImage(mButtonRenderData.image());
        if (button) {
            renderData.setSubLayer(1);
            Vector2 buttonPos { pos.x + currentSize.x, pos.y };
            Vector3 buttonSize { size.y, size.y, size.z };
            mButtonRenderData.renderImage(renderData, buttonPos, buttonSize, *button, color.frame(buttonPos, buttonSize.xy()));
        }

        if (mOpen && mTextRenderData.available()) {
            Vector2i clientSpaceSize = manager().getClientSpace().mSize;

            float lineHeight = mTextRenderData.calculateLineHeight(size.z);
            float totalHeight = mItems.size() * (mSpacing + lineHeight) + mSpacing;

            Vector2 dropDownPos { pos.x, pos.y + size.y };
            Vector3 dropDownSize { size.x, totalHeight, size.z };

            if (dropDownPos.y + totalHeight > clientSpaceSize.y) {
                dropDownPos.y = pos.y - totalHeight;
            }

            if (background) {
                renderData.setSubLayer(0);
                mBackgroundRenderData.renderImage(renderData, dropDownPos, dropDownSize, *background);
            }

            Vector2 textPos { dropDownPos.x, dropDownPos.y + mSpacing };
            Vector3 lineSize { size.x, lineHeight, size.z };
            renderData.setSubLayer(2);
            size_t i = 0;
            for (const std::string &item : mItems) {
                if (i == mHoveredIndex) {
                    const Atlas2::Entry *blankEntry = manager().lookUpImage("blank_white");
                    if (blankEntry) {
                        renderData.setSubLayer(1);
                        renderData.renderQuadUV(textPos, lineSize.xy(), mSelectionColorRenderData.frame(textPos, lineSize.xy()), {}, blankEntry->mArea, { 2048, 2048 }, blankEntry->mFlipped);
                        renderData.setSubLayer(2);
                    }
                }
                ++i;
                mTextRenderData.render(renderData, item, textPos, lineSize);
                textPos.y += lineHeight + mSpacing;
            }
        }
    }

    Execution::SignalStub<size_t> &Combobox::indexChangedEvent()
    {
        return mIndexChanged;
    }

    void Combobox::addItem(const std::string &text)
    {
        mItems.push_back(text);
    }

    void Combobox::clear()
    {
        mItems.clear();
    }

    void Combobox::setCurrentIndex(size_t index)
    {
        if (mSelectedIndex != index) {
            mSelectedIndex = index;
            mIndexChanged.emit(index);
        }
    }

    void Combobox::setEnabled(bool enabled)
    {
        mEnabled = enabled;
    }

    bool Combobox::isEnabled() const
    {
        return mEnabled;
    }

    bool Combobox::containsPoint(const Vector2 &point, const Rect2i &screenSpace, float extend) const
    {
        Vector3 absoluteSize = getAbsoluteSize();
        Vector2 absolutePos = getAbsolutePosition();

        if (mOpen) {
            float additionalHeight = mItems.size() * (mTextRenderData.calculateLineHeight(absoluteSize.z) + mSpacing) + mSpacing;
            absoluteSize.y += additionalHeight;

            if (absolutePos.y + absoluteSize.y > screenSpace.mSize.y)
                absolutePos.y -= additionalHeight;
        }

        Vector2 min = absolutePos + Vector2 { screenSpace.mTopLeft } - extend;
        Vector2 max = absoluteSize.xy() + min + 2 * extend;
        return min.x <= point.x && min.y <= point.y && max.x >= point.x && max.y >= point.y;
    }

    void Combobox::injectPointerEnter(const Input::PointerMoveEvent &arg)
    {
        mHovered = true;
        WidgetBase::injectPointerEnter(arg);
    }

    void Combobox::injectPointerLeave(const Input::PointerMoveEvent &arg)
    {
        mHovered = false;
        mHoveredIndex = -1;
        WidgetBase::injectPointerLeave(arg);
    }

    void Combobox::injectPointerMove(const Input::PointerMoveEvent &arg)
    {

        Vector3 size = getAbsoluteSize();
        Vector2 pos = getAbsolutePosition();

        if (mOpen) {
            Vector2i clientSpaceSize = manager().getClientSpace().mSize;

            float lineHeight = mTextRenderData.calculateLineHeight(size.z);
            float totalHeight = mItems.size() * (mSpacing + lineHeight) + mSpacing;

            float baseY = size.y;

            if (pos.y + size.y + totalHeight > clientSpaceSize.y) {
                baseY -= size.y + totalHeight;
            }
            mHoveredIndex = (arg.mWindowPosition.y - baseY) / (mSpacing + lineHeight);
            LOG(mHoveredIndex);
            if (mHoveredIndex < 0 || mHoveredIndex >= mItems.size())
                mHoveredIndex = -1;
        }

        WidgetBase::injectPointerMove(arg);
    }

    void Combobox::injectPointerClick(const PointerClickEvent &arg)
    {
        if (mEnabled) {
            if (mOpen) {
                if (mHoveredIndex >= 0) {
                    setCurrentIndex(mHoveredIndex);
                }
            }
            mOpen = !mOpen;
            if (mOpen) {

                mHoveredIndex = -1;
            }
        }
        WidgetBase::injectPointerClick(arg);
    }

    void Combobox::onFocusLost()
    {
        mOpen = false;
        mHoveredIndex = -1;
        WidgetBase::onFocusLost();
    }

}
}