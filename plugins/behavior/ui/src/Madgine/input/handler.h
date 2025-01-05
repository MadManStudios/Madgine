#pragma once

#include "Meta/keyvalue/virtualscope.h"
#include "Meta/serialize/hierarchy/virtualserializableunit.h"
#include "Modules/threading/madgineobject.h"

#include "Modules/uniquecomponent/uniquecomponent.h"

#include "handlercollector.h"

#include "Interfaces/input/inputevents.h"

#include "Madgine/widgets/button.h"

#include "Generic/execution/lifetime.h"

namespace Engine {
namespace Input {

    struct MADGINE_UI_EXPORT HandlerBase : VirtualScopeBase<>, Threading::MadgineObject<HandlerBase> {
        SERIALIZABLEUNIT(HandlerBase)

        enum class WidgetType {
            DEFAULT_WIDGET,
            MODAL_OVERLAY,
            NONMODAL_OVERLAY,
            ROOT_WIDGET
        };

        HandlerBase(UIManager &ui, std::string_view widgetName, WidgetType type = WidgetType::DEFAULT_WIDGET);
        virtual ~HandlerBase() = default;

        virtual void onMouseVisibilityChanged(bool b);
                                                                
        virtual void startLifetime();
        void endLifetime();

        Widgets::WidgetBase *widget() const;
        void setWidget(Widgets::WidgetBase *w);

        virtual void sizeChanged();

        virtual std::string_view key() const = 0;

        template <typename T>
        T &getHandler()
        {
            return static_cast<T &>(getHandler(UniqueComponent::component_index<T>()));
        }

        HandlerBase &getHandler(size_t i);

        Threading::TaskQueue *viewTaskQueue() const;
        Threading::TaskQueue *modelTaskQueue() const;

        virtual void open();
        virtual void close();
        bool isOpen() const;

        bool isRootWindow() const;

    protected:
        virtual Threading::Task<bool> init();
        virtual Threading::Task<void> finalize();

        friend struct MadgineObject<HandlerBase>;

        virtual void onPointerMove(const PointerEventArgs &me);
        virtual void onPointerClick(const PointerEventArgs &me);

        virtual void onDragBegin(const PointerEventArgs &me);
        virtual void onDragMove(const PointerEventArgs &me);
        virtual void onDragEnd(const PointerEventArgs &me);
        virtual void onDragAbort();

        virtual bool onKeyPress(const KeyEventArgs &evt);

        virtual void onAxisEvent(const AxisEventArgs &evt);

        bool dragging() const;

        template <typename... Ty>
        Widgets::Button *setupButton(std::string_view name, Ty &&...args)
        {
            Widgets::Button *button = mWidget->getChildRecursive<Widgets::Button>(name);
            if (button)
                mLifetime.attach(button->clickEvent().connect(std::forward<Ty>(args)...));
            return button;
        }

    public:
        void injectPointerMove(const PointerEventArgs &evt);
        void injectPointerClick(const PointerEventArgs &evt);
        void injectDragBegin(const PointerEventArgs &evt);
        void injectDragMove(const PointerEventArgs &evt);
        void injectDragEnd(const PointerEventArgs &evt);
        void injectDragAbort();
        bool injectKeyPress(const KeyEventArgs &evt);
        void injectAxisEvent(const AxisEventArgs &evt);

        void abortDrag();

    protected:
        std::string_view mWidgetName;
        Widgets::WidgetBase *mWidget = nullptr;

        UIManager &mUI;

        DEBUGGABLE_LIFETIME(mLifetime);

        const WidgetType mType;
    };
}
}

REGISTER_TYPE(Engine::Input::HandlerBase)