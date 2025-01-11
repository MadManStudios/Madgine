#include "../uilib.h"
#include "Madgine/widgets/widget.h"
#include "Madgine/widgets/widgetmanager.h"
#include "Madgine/window/mainwindow.h"
#include "widgethandler.h"
#include "uimanager.h"

#include "Meta/keyvalue/metatable_impl.h"

#include "Modules/threading/awaitables/awaitablesender.h"

METATABLE_BEGIN_BASE(Engine::Input::WidgetHandlerBase, Engine::Input::HandlerBase)
PROPERTY(Widget, widget, setWidget)
METATABLE_END(Engine::Input::WidgetHandlerBase)

namespace Engine {
namespace Input {
    WidgetHandlerBase::WidgetHandlerBase(UIManager &ui, std::string_view widgetName, WidgetType type)
        : HandlerBase(ui)
        , mWidgetName(widgetName)
        , mType(type)
    {
    }

    void WidgetHandlerBase::sizeChanged()
    {
    }

    void WidgetHandlerBase::setWidget(Widgets::WidgetBase *widget)
    {
        mWidget = widget;
    }

    void WidgetHandlerBase::injectPointerMove(const PointerEventArgs &evt)
    {
        onPointerMove(evt);
    }

    void WidgetHandlerBase::injectPointerClick(const PointerEventArgs &evt)
    {
        onPointerClick(evt);
    }

    void WidgetHandlerBase::injectDragBegin(const PointerEventArgs &evt)
    {
        onDragBegin(evt);
    }

    void WidgetHandlerBase::injectDragMove(const PointerEventArgs &evt)
    {
        onDragMove(evt);
    }

    void WidgetHandlerBase::injectDragEnd(const PointerEventArgs &evt)
    {
        onDragEnd(evt);
    }

    void WidgetHandlerBase::injectDragAbort()
    {
        onDragAbort();
    }

    bool WidgetHandlerBase::injectKeyPress(const KeyEventArgs &evt)
    {
        return onKeyPress(evt);
    }

    void WidgetHandlerBase::injectAxisEvent(const AxisEventArgs &evt)
    {
        onAxisEvent(evt);
    }

    void WidgetHandlerBase::abortDrag()
    {
        if (mWidget)
            mWidget->abortDrag();
    }

    void WidgetHandlerBase::onPointerMove(const PointerEventArgs &me)
    {
    }

    void WidgetHandlerBase::onPointerClick(const PointerEventArgs &me)
    {
    }

    void WidgetHandlerBase::onDragBegin(const PointerEventArgs &me)
    {
    }

    void WidgetHandlerBase::onDragMove(const PointerEventArgs &me)
    {
    }

    void WidgetHandlerBase::onDragEnd(const PointerEventArgs &me)
    {
    }

    void WidgetHandlerBase::onDragAbort()
    {
    }

    bool WidgetHandlerBase::onKeyPress(const KeyEventArgs &evt)
    {
        return false;
    }

    void WidgetHandlerBase::onAxisEvent(const AxisEventArgs &evt)
    {
    }

    bool WidgetHandlerBase::dragging() const
    {
        return mWidget ? mWidget->dragging() : false;
    }

    void WidgetHandlerBase::startLifetime()
    {
        HandlerBase::startLifetime();

        mWidget = mUI.window().getWindowComponent<Widgets::WidgetManager>().getWidget(mWidgetName);

        if (mWidget) {
            mLifetime.attach(mWidget->pointerMoveEvent().connect(&WidgetHandlerBase::injectPointerMove, this));
            mLifetime.attach(mWidget->pointerClickEvent().connect(&WidgetHandlerBase::injectPointerClick, this));
            mLifetime.attach(mWidget->dragBeginEvent().connect(&WidgetHandlerBase::injectDragBegin, this));
            mLifetime.attach(mWidget->dragMoveEvent().connect(&WidgetHandlerBase::injectDragMove, this));
            mLifetime.attach(mWidget->dragEndEvent().connect(&WidgetHandlerBase::injectDragEnd, this));
            mLifetime.attach(mWidget->axisEvent().connect(&WidgetHandlerBase::injectAxisEvent, this));
            mLifetime.attach(mWidget->keyEvent().connect(&WidgetHandlerBase::injectKeyPress, this));
            mWidget->setAcceptsPointerEvents(true);
        }
    }

    Widgets::WidgetBase *WidgetHandlerBase::widget() const
    {
        return mWidget;
    }

    void WidgetHandlerBase::open()
    {
        assert(mType != WidgetType::DEFAULT_WIDGET);

        if (!mWidget)
            return;

        auto state = this->state();
        if (!state.is_ready() || !state) {
            LOG_ERROR("Failed to open unitialized Handler!");
            return;
        }

        if (isOpen())
            return;

        switch (mType) {
        case WidgetType::MODAL_OVERLAY:
            mWidget->manager().openModalWidget(mWidget);
            break;
        case WidgetType::NONMODAL_OVERLAY:
            mWidget->manager().openWidget(mWidget);
            break;
        case WidgetType::ROOT_WIDGET:
            mWidget->manager().swapCurrentRoot(mWidget);
            break;
        }
    }

    void WidgetHandlerBase::close()
    {
        assert(mType != WidgetType::DEFAULT_WIDGET);

        switch (mType) {
        case WidgetType::MODAL_OVERLAY:
            mWidget->manager().closeModalWidget(mWidget);
            break;
        case WidgetType::NONMODAL_OVERLAY:
            mWidget->manager().closeWidget(mWidget);
            break;
        case WidgetType::ROOT_WIDGET:
            std::terminate();
        }
    }

    bool WidgetHandlerBase::isOpen() const
    {
        return mWidget->mVisible;
    }

    bool WidgetHandlerBase::isRootWindow() const
    {
        return mType == WidgetType::ROOT_WIDGET;
    }

}
}
