#pragma once

#include "Generic/execution/signal.h"
#include "Generic/projections.h"

#include "Platform/log/logsenders.h"

#include "Meta/math/matrix3.h"
#include "Meta/reflect/virtualscope.h"
#include "Meta/serialize/configs/creator.h"
#include "Meta/serialize/hierarchy/virtualserializableunit.h"

#include "Madgine/behavior/context.h"
#include "Madgine/debug/debuggablelifetime.h"
#include "Madgine/debug/debuggablesender.h"
#include "Madgine/render/texturedescriptor.h"

#include "condition.h"
#include "properties.h"
#include "util/widgetsrenderdata.h"
#include "widgetcollector.h"
#include "widgetloader.h"

namespace Engine {
namespace Widgets {

    struct WidgetConfig {
        bool acceptsPointerEvents = false;
        bool allowsDragging = false;
    };

    struct MADGINE_WIDGETS_EXPORT WidgetBase : Reflect::VirtualScope<WidgetBase, Serialize::VirtualData<WidgetBase, Serialize::VirtualSerializableDataBase<Reflect::VirtualScopeBase<>>>> {
        SERIALIZABLEUNIT(WidgetBase)

        WidgetBase(WidgetManager &manager, WidgetBase *parent = nullptr, const WidgetConfig &config = {});

        WidgetBase(const WidgetBase &) = delete;

        virtual ~WidgetBase();

        virtual const char *getClass() const;

        WidgetManager &manager();
        const std::string &key() const;

        void destroy();

        void show();
        void hide();
        void setVisible(bool v);

        void setSize(const Math::Matrix3 &size);
        const Math::Matrix3 &getSize();
        void setPos(const Math::Matrix3 &pos);
        const Math::Matrix3 &getPos() const;

        void setOpacity(float opacity);
        float opacity() const;

        Math::Vector3 getAbsoluteSize() const;
        Math::Vector2 getAbsolutePosition() const;
        void setAbsoluteSize(const Math::Vector3 &size);
        void setAbsolutePosition(const Math::Vector2 &pos);

        void applyGeometry();
        void applyGeometry(const Math::Vector3 &parentSize, const Math::Vector2 &parentPos = Math::Vector2::ZERO);
        Geometry getGeometry();

        template <typename WidgetType = WidgetBase>
        WidgetType *createChild()
        {
            return static_cast<WidgetType *>(mChildren.emplace_back(std::make_unique<WidgetType>(mManager, this)).get());
        }
        WidgetBase *createChildByDescriptor(const WidgetLoader::Handle &desc);
        WidgetBase *createChildByAnnotation(const WidgetRegistry::Annotations &annotation);
        void clearChildren();

        virtual WidgetBase *getChildRecursive(std::string_view name);
        template <typename T>
        T *getChildRecursive(std::string_view name)
        {
            return dynamic_cast<T *>(getChildRecursive(name));
        }

        decltype(auto) children() const
        {
            return mChildren | std::views::transform(projectionUniquePtrToPtr);
        }

        void setParent(WidgetBase *parent);
        WidgetBase *getParent() const;

        void setAcceptsPointerEvents(bool v);
        bool acceptsPointerEvents() const;

        void setAllowsDragging(bool v);
        bool allowsDragging() const;

        bool isFocused() const;
        virtual void onFocusLost();

        bool dragging() const;
        void abortDrag();

        virtual void injectPointerClick(const PointerClickEvent &arg);
        virtual void injectPointerMove(const Platform::Input::PointerMoveEvent &arg);
        virtual void injectPointerEnter(const Platform::Input::PointerMoveEvent &arg);
        virtual void injectPointerLeave(const Platform::Input::PointerMoveEvent &arg);
        virtual void injectDragBegin(const DragBeginEvent &arg);
        virtual void injectDragMove(const DragMoveEvent &arg);
        virtual void injectDragEnd(const DragEndEvent &arg);
        virtual void injectDragAbort();
        virtual bool injectAxisEvent(const Platform::Input::AxisEvent &arg);
        virtual bool injectKeyPress(const Platform::Input::KeyPressEvent &arg);
        virtual bool injectKeyRelease(const Platform::Input::KeyReleaseEvent &arg);

        Execution::SignalStub<void, const Platform::Input::PointerMoveEvent &> &pointerMoveEvent();
        Execution::SignalStub<void, const PointerClickEvent &> &pointerClickEvent();
        Execution::SignalStub<void, const Platform::Input::PointerMoveEvent &> &pointerEnterEvent();
        auto pointerEnterSender()
        {
            return mPointerEnterSignal | Execution::then([](const Platform::Input::PointerMoveEvent &args) {
                return 3;
            });
        }
        Execution::SignalStub<void, const Platform::Input::PointerMoveEvent &> &pointerLeaveEvent();
        auto pointerLeaveSender()
        {
            return mPointerLeaveSignal | Execution::then([](const Platform::Input::PointerMoveEvent &args) {
                return 3;
            });
        }
        Execution::SignalStub<void, const DragBeginEvent &> &dragBeginEvent();
        Execution::SignalStub<void, const DragMoveEvent &> &dragMoveEvent();
        Execution::SignalStub<void, const DragEndEvent &> &dragEndEvent();
        Execution::SignalStub<void> &dragAbortEvent();
        Execution::SignalStub<void, const Platform::Input::AxisEvent &> &axisEvent();
        Execution::SignalStub<void, const Platform::Input::KeyPressEvent &> &keyPressEvent();
        Execution::SignalStub<void, const Platform::Input::KeyReleaseEvent &> &keyReleaseEvent();

        virtual bool containsPoint(const Math::Vector2 &point, const Math::Rect2i &screenSpace, float extend = 0.0f) const;
        WidgetBase *getHoveredUp(const Math::Vector2 &point, const Math::Rect2i &screenSpace);
        virtual WidgetBase *getHoveredDown(const Math::Vector2 &point, const Math::Rect2i &screenSpace);

        virtual void render(WidgetsRenderData &renderData);

        uint16_t fetchActiveConditions(std::vector<Condition *> *conditions = nullptr);

        Geometry calculateGeometry(uint16_t activeConditions, GeometrySourceInfo *source = nullptr);

        void addConditional(uint16_t mask);
        void removeConditional(uint16_t mask);
        PropertyRange conditionals();

        void setPosValue(uint16_t index, float value, uint16_t mask = 0);
        void unsetPosValue(uint16_t index, uint16_t mask);

        void setSizeValue(uint16_t index, float value, uint16_t mask = 0);
        void unsetSizeValue(uint16_t index, uint16_t mask);

        template <typename Sender>
        void addBehavior(Sender &&sender)
        {
            lifetime().attach(std::forward<Sender>(sender) | Behavior::context_set(this) | Platform::Log::log_result());
        }
        Debug::DebuggableLifetime<Reflect::get_reflect_contextual> &lifetime();

        bool mVisible = true;
        std::string mName = "Unnamed";

        std::vector<Condition> mConditions;

    protected:
        Serialize::StreamResult readWidget(Serialize::FormattedSerializeStream &in, std::unique_ptr<WidgetBase> &widget);
        const char *writeWidget(Serialize::FormattedSerializeStream &out, const std::unique_ptr<WidgetBase> &widget) const;
        static Serialize::StreamResult scanWidget(const Serialize::SerializeTable *&out, Serialize::FormattedSerializeStream &in);

        virtual void sizeChanged(const Math::Vector3 &pixelSize);

        uint16_t fetchActiveConditionsImpl(std::vector<Condition *> &conditions);

        bool evalCondition(Condition &cond);

        virtual void updateChildrenGeometry();

    protected:
        void destroyChild(WidgetBase *w);

        Execution::Signal<void, const Platform::Input::PointerMoveEvent &> mPointerMoveSignal;
        Execution::Signal<void, const PointerClickEvent &> mPointerClickSignal;
        Execution::Signal<void, const Platform::Input::PointerMoveEvent &> mPointerEnterSignal;
        Execution::Signal<void, const Platform::Input::PointerMoveEvent &> mPointerLeaveSignal;
        Execution::Signal<void, const DragBeginEvent &> mDragBeginSignal;
        Execution::Signal<void, const DragMoveEvent &> mDragMoveSignal;
        Execution::Signal<void, const DragEndEvent &> mDragEndSignal;
        Execution::Signal<void> mDragAbortSignal;
        Execution::Signal<void, const Platform::Input::AxisEvent &> mAxisEventSignal;
        Execution::Signal<void, const Platform::Input::KeyPressEvent &> mKeyPressSignal;
        Execution::Signal<void, const Platform::Input::KeyReleaseEvent &> mKeyReleaseSignal;

        std::vector<std::unique_ptr<WidgetBase>> mChildren;

    private:
        WidgetManager &mManager;

        WidgetBase *mParent;

        Math::Vector2 mAbsolutePos;
        Math::Vector3 mAbsoluteSize;

        float mOpacity = 1.0f;

        bool mAcceptsPointerEvents;
        bool mAllowsDragging;

        PropertyList mProperties;

        Math::Matrix3 mPos = Math::Matrix3::ZERO;
        Math::Matrix3 mSize = Math::Matrix3::IDENTITY;

    public:
        using WidgetCreator = Serialize::ParentCreator<&WidgetBase::readWidget, &WidgetBase::writeWidget, nullptr, &WidgetBase::scanWidget>;
    };

    template <typename T>
    struct Widget : Reflect::VirtualScope<T, Serialize::VirtualData<T, WidgetComponent<T, WidgetBase>>> {

        using Reflect::VirtualScope<T, Serialize::VirtualData<T, WidgetComponent<T, WidgetBase>>>::VirtualScope;

        const char *getClass() const final
        {
            return T::componentName().data();
        }
    };
}
}
