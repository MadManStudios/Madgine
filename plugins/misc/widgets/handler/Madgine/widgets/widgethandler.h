#pragma once

#include "Madgine/handlercollector.h"

#include "Interfaces/input/inputevents.h"

#include "Madgine/widgets/button.h"

#include "Madgine/handler.h"

namespace Engine {
namespace Widgets {

    struct MADGINE_WIDGETHANDLER_EXPORT WidgetHandlerBase : HandlerBase {
        SERIALIZABLEUNIT(WidgetHandlerBase)

        enum class WidgetType {
            DEFAULT_WIDGET,
            MODAL_OVERLAY,
            NONMODAL_OVERLAY,
            ROOT_WIDGET
        };

        WidgetHandlerBase(HandlerManager &ui, std::string_view widgetName, WidgetType type = WidgetType::DEFAULT_WIDGET);
        virtual ~WidgetHandlerBase() = default;

        virtual void startLifetime();

        Widgets::WidgetBase *widget() const;
        virtual void setWidget(Widgets::WidgetBase *w);

        virtual void sizeChanged();

        virtual void open();
        virtual void close();
        bool isOpen() const;

        bool isRootWindow() const;

    protected:
        virtual void onPointerMove(const Input::PointerEventArgs &me);
        virtual void onPointerClick(const Input::PointerEventArgs &me);

        virtual void onDragBegin(const Input::PointerEventArgs &me);
        virtual void onDragMove(const Input::PointerEventArgs &me);
        virtual void onDragEnd(const Input::PointerEventArgs &me);
        virtual void onDragAbort();

        virtual bool onKeyPress(const Input::KeyEventArgs &evt);

        virtual void onAxisEvent(const Input::AxisEventArgs &evt);

        bool dragging() const;

        void abortDrag();

        template <typename... Ty>
        Widgets::Button *setupButton(std::string_view name, Ty &&...args)
        {
            if (!mWidget)
                return nullptr;
            Widgets::Button *button = mWidget->getChildRecursive<Widgets::Button>(name);
            if (button)
                mLifetime.attach(button->clickEvent().connect(std::forward<Ty>(args)...));
            return button;
        }

    protected:
        std::string_view mWidgetName;
        Widgets::WidgetBase *mWidget = nullptr;

        const WidgetType mType;
    };

    
    template <typename T>
    using WidgetHandler = VirtualScope<T, HandlerComponent<T, WidgetHandlerBase>>;
}
}

REGISTER_TYPE(Engine::Widgets::WidgetHandlerBase)