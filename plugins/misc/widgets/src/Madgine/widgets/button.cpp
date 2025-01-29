#include "../widgetslib.h"

#include "button.h"

#include "widgetmanager.h"

#include "Meta/math/vector4.h"

#include "Meta/keyvalue/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

#include "Madgine/fontloader/fontloader.h"

#include "Madgine/imageloader/imageloader.h"

METATABLE_BEGIN_BASE(Engine::Widgets::Button, Engine::Widgets::WidgetBase)
MEMBER(mText)
NAMED_MEMBER(TextData, mTextRenderData)
NAMED_MEMBER(Image, mImageRenderData)
NAMED_MEMBER(ColorTint, mColorTintRenderData)
MEMBER(mEnabled)
METATABLE_END(Engine::Widgets::Button)

SERIALIZETABLE_INHERIT_BEGIN(Engine::Widgets::Button, Engine::Widgets::WidgetBase)
FIELD(mText)
FIELD(mTextRenderData)
FIELD(mImageRenderData)
FIELD(mColorTintRenderData)
FIELD(mEnabled)
SERIALIZETABLE_END(Engine::Widgets::Button)

namespace Engine {
namespace Widgets {

    Button::Button(WidgetManager &manager, WidgetBase *parent)
        : Widget(manager, parent, { .acceptsPointerEvents = true })
    {
    }

    Execution::SignalStub<> &Button::clickEvent()
    {
        return mClicked;
    }

    void Button::setEnabled(bool enabled)
    {
        mEnabled = enabled;
    }

    bool Button::isEnabled() const
    {
        return mEnabled;
    }

    void Button::vertices(WidgetsRenderData &renderData, size_t layer)
    {
        const Atlas2::Entry *entry = manager().lookUpImage(mImageRenderData.image());

        Vector3 pos { getAbsolutePosition(), static_cast<float>(depth(layer)) };
        Vector3 size = getAbsoluteSize();

        Color4 color = mEnabled ? (mHovered ? mColorTintRenderData.mHighlightedColor : mColorTintRenderData.mNormalColor)
                                : mColorTintRenderData.mDisabledColor;

        if (entry) {
            mImageRenderData.renderImage(renderData, pos, size.xy(), *entry, color);
        }

        if (mTextRenderData.available()) {
            mTextRenderData.render(renderData, mText, pos, size);
        }
    }

    void Button::injectPointerEnter(const Input::PointerEventArgs &arg)
    {
        mHovered = true;
        WidgetBase::injectPointerEnter(arg);
    }

    void Button::injectPointerLeave(const Input::PointerEventArgs &arg)
    {
        mHovered = false;
        WidgetBase::injectPointerLeave(arg);
    }

    void Button::injectPointerClick(const Input::PointerEventArgs &arg)
    {
        if (mEnabled)
            emitClicked();
        WidgetBase::injectPointerClick(arg);
    }

    void Button::emitClicked()
    {
        mClicked.emit();
    }

    WidgetClass Button::getClass() const
    {
        return WidgetClass::BUTTON;
    }

}
}