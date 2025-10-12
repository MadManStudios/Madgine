#include "../widgethandlerlib.h"
#include "widgethandler.h"
#include "Madgine/handlermanager.h"
#include "Madgine/widgets/widget.h"
#include "Madgine/widgets/widgetmanager.h"
#include "Madgine/window/mainwindow.h"

#include "Meta/keyvalue/metatable_impl.h"

#include "Modules/threading/awaitables/awaitablesender.h"

METATABLE_BEGIN_BASE(Engine::Widgets::WidgetHandlerBase, Engine::HandlerBase)
PROPERTY(Widget, widget, setWidget)
METATABLE_END(Engine::Widgets::WidgetHandlerBase)

namespace Engine {
namespace Widgets {
    WidgetHandlerBase::WidgetHandlerBase(HandlerManager &ui, std::string_view widgetName, WidgetType type)
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

    void WidgetHandlerBase::abortDrag()
    {
        if (mWidget)
            mWidget->abortDrag();
    }

    void WidgetHandlerBase::onPointerMove(const Input::PointerMoveEvent &me)
    {
    }

    void WidgetHandlerBase::onPointerClick(const PointerClickEvent &me)
    {
    }

    void WidgetHandlerBase::onDragBegin(const DragBeginEvent &me)
    {
    }

    void WidgetHandlerBase::onDragMove(const DragMoveEvent &me)
    {
    }

    void WidgetHandlerBase::onDragEnd(const DragEndEvent &me)
    {
    }

    void WidgetHandlerBase::onDragAbort()
    {
    }

    bool WidgetHandlerBase::onKeyPress(const Input::KeyPressEvent &evt)
    {
        return false;
    }

    bool WidgetHandlerBase::onKeyRelease(const Input::KeyReleaseEvent &evt)
    {
        return false;
    }

    void WidgetHandlerBase::onAxisEvent(const Input::AxisEvent &evt)
    {
    }

    bool WidgetHandlerBase::dragging() const
    {
        return mWidget ? mWidget->dragging() : false;
    }

    void WidgetHandlerBase::startLifetime()
    {
        HandlerBase::startLifetime();

        Widgets::WidgetBase *widget = mUI.window().getWindowComponent<Widgets::WidgetManager>().getWidget(mWidgetName);
        if (widget != mWidget)
            setWidget(widget);

        if (mWidget) {
            mLifetime.attach(mWidget->pointerMoveEvent().connect(&WidgetHandlerBase::onPointerMove, this));
            mLifetime.attach(mWidget->pointerClickEvent().connect(&WidgetHandlerBase::onPointerClick, this));
            mLifetime.attach(mWidget->dragBeginEvent().connect(&WidgetHandlerBase::onDragBegin, this));
            mLifetime.attach(mWidget->dragMoveEvent().connect(&WidgetHandlerBase::onDragMove, this));
            mLifetime.attach(mWidget->dragEndEvent().connect(&WidgetHandlerBase::onDragEnd, this));
            mLifetime.attach(mWidget->axisEvent().connect(&WidgetHandlerBase::onAxisEvent, this));
            mLifetime.attach(mWidget->keyPressEvent().connect(&WidgetHandlerBase::onKeyPress, this));
            mLifetime.attach(mWidget->keyReleaseEvent().connect(&WidgetHandlerBase::onKeyRelease, this));
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
