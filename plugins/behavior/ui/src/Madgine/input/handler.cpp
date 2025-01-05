#include "../uilib.h"
#include "handler.h"
#include "Madgine/widgets/widget.h"
#include "Madgine/widgets/widgetmanager.h"
#include "Madgine/window/mainwindow.h"
#include "uimanager.h"

#include "Meta/keyvalue/metatable_impl.h"

#include "Modules/threading/awaitables/awaitablesender.h"

DEFINE_UNIQUE_COMPONENT(Engine::Input, Handler)

METATABLE_BEGIN(Engine::Input::HandlerBase)
PROPERTY(Widget, widget, setWidget)
METATABLE_END(Engine::Input::HandlerBase)

namespace Engine {
namespace Input {
    HandlerBase::HandlerBase(UIManager &ui, std::string_view widgetName, WidgetType type)
        : mWidgetName(widgetName)
        , mUI(ui)
        , mType(type)
        , mLifetime(&ui.lifetime())
    {
    }

    HandlerBase &HandlerBase::getHandler(size_t i)
    {
        return mUI.getHandler(i);
    }

    void HandlerBase::sizeChanged()
    {
    }

    Threading::Task<bool> HandlerBase::init()
    {
        co_return true;
    }

    Threading::Task<void> HandlerBase::finalize()
    {
        mWidget = nullptr;
        co_return;
    }

    void HandlerBase::setWidget(Widgets::WidgetBase *widget)
    {
        mWidget = widget;
    }

    void HandlerBase::injectPointerMove(const PointerEventArgs &evt)
    {
        onPointerMove(evt);
    }

    void HandlerBase::injectPointerClick(const PointerEventArgs &evt)
    {
        onPointerClick(evt);
    }

    void HandlerBase::injectDragBegin(const PointerEventArgs &evt)
    {
        onDragBegin(evt);
    }

    void HandlerBase::injectDragMove(const PointerEventArgs &evt)
    {
        onDragMove(evt);
    }

    void HandlerBase::injectDragEnd(const PointerEventArgs &evt)
    {
        onDragEnd(evt);
    }

    void HandlerBase::injectDragAbort()
    {
        onDragAbort();
    }

    bool HandlerBase::injectKeyPress(const KeyEventArgs &evt)
    {
        return onKeyPress(evt);
    }

    void HandlerBase::injectAxisEvent(const AxisEventArgs &evt)
    {
        onAxisEvent(evt);
    }

    void HandlerBase::abortDrag()
    {
        if (mWidget)
            mWidget->abortDrag();
    }

    void HandlerBase::onPointerMove(const PointerEventArgs &me)
    {
    }

    void HandlerBase::onPointerClick(const PointerEventArgs &me)
    {
    }

    void HandlerBase::onDragBegin(const PointerEventArgs &me)
    {
    }

    void HandlerBase::onDragMove(const PointerEventArgs &me)
    {
    }

    void HandlerBase::onDragEnd(const PointerEventArgs &me)
    {
    }

    void HandlerBase::onDragAbort()
    {
    }

    bool HandlerBase::onKeyPress(const KeyEventArgs &evt)
    {
        return false;
    }

    void HandlerBase::onAxisEvent(const AxisEventArgs &evt)
    {
    }

    bool HandlerBase::dragging() const
    {
        return mWidget ? mWidget->dragging() : false;
    }

    void HandlerBase::onMouseVisibilityChanged(bool b)
    {
    }

    void HandlerBase::startLifetime()
    {
        mUI.lifetime().attach(mLifetime);

        mWidget = mUI.window().getWindowComponent<Widgets::WidgetManager>().getWidget(mWidgetName);

        if (mWidget) {
            mLifetime.attach(mWidget->pointerMoveEvent().connect(&HandlerBase::injectPointerMove, this));
            mLifetime.attach(mWidget->pointerClickEvent().connect(&HandlerBase::injectPointerClick, this));
            mLifetime.attach(mWidget->dragBeginEvent().connect(&HandlerBase::injectDragBegin, this));
            mLifetime.attach(mWidget->dragMoveEvent().connect(&HandlerBase::injectDragMove, this));
            mLifetime.attach(mWidget->dragEndEvent().connect(&HandlerBase::injectDragEnd, this));
            mLifetime.attach(mWidget->axisEvent().connect(&HandlerBase::injectAxisEvent, this));
            mLifetime.attach(mWidget->keyEvent().connect(&HandlerBase::injectKeyPress, this));
            mWidget->setAcceptsPointerEvents(true);
        }
    }

    void HandlerBase::endLifetime()
    {
        mLifetime.end();
    }

    Widgets::WidgetBase *HandlerBase::widget() const
    {
        return mWidget;
    }

    Threading::TaskQueue *HandlerBase::viewTaskQueue() const
    {
        return mUI.viewTaskQueue();
    }

    void HandlerBase::open()
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

    void HandlerBase::close()
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

    bool HandlerBase::isOpen() const
    {
        return mWidget->mVisible;
    }

    bool HandlerBase::isRootWindow() const
    {
        return mType == WidgetType::ROOT_WIDGET;
    }

}
}
