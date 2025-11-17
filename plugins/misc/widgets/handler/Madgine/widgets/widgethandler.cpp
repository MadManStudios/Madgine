#include "../widgethandlerlib.h"
#include "widgethandler.h"
#include "Madgine/handlermanager.h"
#include "Madgine/widgets/widget.h"
#include "Madgine/widgets/widgetmanager.h"
#include "Madgine/window/mainwindow.h"

#include "Meta/keyvalue/metatable_impl.h"

#include "Modules/threading/awaitables/awaitablesender.h"

METATABLE_BEGIN_BASE(Engine::Widgets::WidgetHandlerBase, Engine::Behavior::HandlerBase)
PROPERTY(Widget, widget, setWidget)
METATABLE_END(Engine::Widgets::WidgetHandlerBase)

namespace Engine {
namespace Widgets {
    WidgetHandlerBase::WidgetHandlerBase(Behavior::HandlerManager &ui, std::string_view widgetName)
        : HandlerBase(ui)
        , mWidgetName(widgetName)
    {
    }

    void WidgetHandlerBase::sizeChanged()
    {
    }

    void WidgetHandlerBase::setWidget(Widgets::WidgetBase *widget)
    {
        if (widget){
            mLifetime.attach(widget->pointerMoveEvent().connect(&WidgetHandlerBase::onPointerMove, this));
            mLifetime.attach(widget->pointerClickEvent().connect(&WidgetHandlerBase::onPointerClick, this));
            mLifetime.attach(widget->dragBeginEvent().connect(&WidgetHandlerBase::onDragBegin, this));
            mLifetime.attach(widget->dragMoveEvent().connect(&WidgetHandlerBase::onDragMove, this));
            mLifetime.attach(widget->dragEndEvent().connect(&WidgetHandlerBase::onDragEnd, this));
            mLifetime.attach(widget->axisEvent().connect(&WidgetHandlerBase::onAxisEvent, this));
            mLifetime.attach(widget->keyPressEvent().connect(&WidgetHandlerBase::onKeyPress, this));
            mLifetime.attach(widget->keyReleaseEvent().connect(&WidgetHandlerBase::onKeyRelease, this));
            widget->setAcceptsPointerEvents(true);
        }
    }

    void WidgetHandlerBase::abortDrag()
    {
        if (mWidget)
            std::get<0>(*mWidget->mWidget)->abortDrag();
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
        return mWidget ? std::get<0>(*mWidget->mWidget)->dragging() : false;
    }

    void WidgetHandlerBase::startLifetime()
    {
        HandlerBase::startLifetime();

        mWidget = mUI.window().getWindowComponent<Widgets::WidgetManager>().getLayoutWidget(mWidgetName);
        if (mWidget)
            mLifetime.attach(mWidget->mWidget | Execution::then([this](WidgetBase *widget) {
                assert(widget);
                setWidget(widget);
            }));
        else
            setWidget(nullptr);

    }

    Widgets::WidgetBase *WidgetHandlerBase::widget() const
    {
        return std::get<0>(*mWidget->mWidget);
    }

    void WidgetHandlerBase::open()
    {
        if (!mWidget)
            return;

        assert(mWidget->mType != WidgetType::DEFAULT_WIDGET);

        auto state = this->state();
        if (!state.is_ready() || !state) {
            LOG_ERROR("Failed to open unitialized Handler!");
            return;
        }

        if (isOpen())
            return;

        mUI.window().getWindowComponent<Widgets::WidgetManager>().openLayout(mWidgetName);
    }

    void WidgetHandlerBase::close()
    {
        assert(mWidget->mType != WidgetType::DEFAULT_WIDGET);

        mUI.window().getWindowComponent<Widgets::WidgetManager>().closeLayout(mWidgetName);
    }

    bool WidgetHandlerBase::isOpen() const
    {
        return mWidget->mWidget.isSet() && std::get<0>(*mWidget->mWidget)->mVisible;
    }

    bool WidgetHandlerBase::isRootWindow() const
    {
        return mWidget->mType == WidgetType::ROOT_WIDGET;
    }

}
}
