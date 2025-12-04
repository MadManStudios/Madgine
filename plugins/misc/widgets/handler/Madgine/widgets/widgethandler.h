#pragma once

#include "Interfaces/input/inputevents.h"

#include "Madgine/handler.h"
#include "Madgine/handlercollector.h"
#include "Madgine/widgets/button.h"
#include "Madgine/widgets/layoutwidget.h"

namespace Engine {
namespace Widgets {

    struct MADGINE_WIDGETHANDLER_EXPORT WidgetHandlerBase : Behavior::HandlerBase {
        SERIALIZABLEUNIT(WidgetHandlerBase)

        WidgetHandlerBase(Behavior::HandlerManager &ui, std::string_view widgetName);
        virtual ~WidgetHandlerBase() = default;

        void startLifetime() final;

        Widgets::WidgetBase *widget() const;
        virtual void setWidget(Widgets::WidgetBase *w);

        virtual void sizeChanged();

        virtual void open();
        virtual void close();
        bool isOpen() const;

        bool isRootWindow() const;

    protected:
        virtual void onPointerMove(const Input::PointerMoveEvent &me);
        virtual void onPointerClick(const PointerClickEvent &me);

        virtual void onDragBegin(const DragBeginEvent &me);
        virtual void onDragMove(const DragMoveEvent &me);
        virtual void onDragEnd(const DragEndEvent &me);
        virtual void onDragAbort();

        virtual bool onKeyPress(const Input::KeyPressEvent &evt);
        virtual bool onKeyRelease(const Input::KeyReleaseEvent &evt);

        virtual void onAxisEvent(const Input::AxisEvent &evt);

        bool dragging() const;

        void abortDrag();

        template <typename... Ty>
        Widgets::Button *setupButton(std::string_view name, Ty &&...args)
        {
            if (!mWidget)
                return nullptr;
            Widgets::Button *button = std::get<0>(*mWidget->mWidget)->getChildRecursive<Widgets::Button>(name);
            if (button)
                mLifetime.attach(button->clickEvent().connect(std::forward<Ty>(args)...));
            return button;
        }

    protected:
        std::string_view mWidgetName;
        Widgets::LayoutWidget *mWidget = nullptr;
    };

    template <typename T>
    using WidgetHandler = VirtualScope<T, Behavior::HandlerComponent<T, WidgetHandlerBase>>;
}
}