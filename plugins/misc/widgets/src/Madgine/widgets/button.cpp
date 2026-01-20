#include "../widgetslib.h"

#include "button.h"

#include "Meta/math/vector4.h"

#include "Madgine/imageloader/imageloader.h"
#include "Madgine/render/fonts/fontloader.h"

#include "Meta/keyvalue/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

#include "widgetmanager.h"

NAMED_UNIQUECOMPONENT(Button, Engine::Widgets::Button);

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

    void Button::render(WidgetsRenderData &renderData)
    {
        const Atlas2::Entry *entry = manager().lookUpImage(mImageRenderData.image());

        Vector2 pos = getAbsolutePosition();
        Vector3 size = getAbsoluteSize();

        const ColorRenderData &color = mEnabled ? (mHovered ? mColorTintRenderData.mHighlightedColor : mColorTintRenderData.mNormalColor)
                                : mColorTintRenderData.mDisabledColor;

        if (entry) {
            mImageRenderData.renderImage(renderData, pos, size.xy(), *entry, color.frame(pos, size.xy()));
        }

        if (mTextRenderData.available()) {
            mTextRenderData.render(renderData, mText.get(), pos, size);
        }

        WidgetBase::render(renderData);
    }

    void Button::injectPointerEnter(const Input::PointerMoveEvent &arg)
    {
        mHovered = true;
        WidgetBase::injectPointerEnter(arg);
    }

    void Button::injectPointerLeave(const Input::PointerMoveEvent &arg)
    {
        mHovered = false;
        WidgetBase::injectPointerLeave(arg);
    }

    void Button::injectPointerClick(const PointerClickEvent &arg)
    {
        if (mEnabled)
            emitClicked();
        WidgetBase::injectPointerClick(arg);
    }

    void Button::emitClicked()
    {
        mClicked.emit();
    }

}
}