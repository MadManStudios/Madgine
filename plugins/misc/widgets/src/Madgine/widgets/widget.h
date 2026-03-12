#pragma once

#include "Generic/execution/signal.h"
#include "Generic/projections.h"

#include "Interfaces/log/logsenders.h"

#include "Meta/keyvalue/virtualscope.h"
#include "Meta/math/matrix3.h"
#include "Meta/serialize/hierarchy/virtualserializableunit.h"

#include "Madgine/behavior/named.h"
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

    struct MADGINE_WIDGETS_EXPORT WidgetBase : VirtualScope<WidgetBase, Serialize::VirtualData<WidgetBase, Serialize::VirtualSerializableDataBase<VirtualScopeBase<>>>> {
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

        void setSize(const Matrix3 &size);
        const Matrix3 &getSize();
        void setPos(const Matrix3 &pos);
        const Matrix3 &getPos() const;

        void setOpacity(float opacity);
        float opacity() const;

        Vector3 getAbsoluteSize() const;
        Vector2 getAbsolutePosition() const;
        void setAbsoluteSize(const Vector3 &size);
        void setAbsolutePosition(const Vector2 &pos);

        void applyGeometry();
        void applyGeometry(const Vector3 &parentSize, const Vector2 &parentPos = Vector2::ZERO);
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
        virtual void injectPointerMove(const Input::PointerMoveEvent &arg);
        virtual void injectPointerEnter(const Input::PointerMoveEvent &arg);
        virtual void injectPointerLeave(const Input::PointerMoveEvent &arg);
        virtual void injectDragBegin(const DragBeginEvent &arg);
        virtual void injectDragMove(const DragMoveEvent &arg);
        virtual void injectDragEnd(const DragEndEvent &arg);
        virtual void injectDragAbort();
        virtual bool injectAxisEvent(const Input::AxisEvent &arg);
        virtual bool injectKeyPress(const Input::KeyPressEvent &arg);
        virtual bool injectKeyRelease(const Input::KeyReleaseEvent &arg);
              
        Execution::SignalStub<void, const Input::PointerMoveEvent &> &pointerMoveEvent();
        Execution::SignalStub<void, const PointerClickEvent &> &pointerClickEvent();
        Execution::SignalStub<void, const Input::PointerMoveEvent &> &pointerEnterEvent();
        auto pointerEnterSender()
        {
            return mPointerEnterSignal | Execution::then([](const Input::PointerMoveEvent &args) {
                return 3;
            });
        }
        Execution::SignalStub<void, const Input::PointerMoveEvent &> &pointerLeaveEvent();
        auto pointerLeaveSender()
        {
            return mPointerLeaveSignal | Execution::then([](const Input::PointerMoveEvent &args) {
                return 3;
            });
        }
        Execution::SignalStub<void, const DragBeginEvent &> &dragBeginEvent();
        Execution::SignalStub<void, const DragMoveEvent &> &dragMoveEvent();
        Execution::SignalStub<void, const DragEndEvent &> &dragEndEvent();
        Execution::SignalStub<void> &dragAbortEvent();
        Execution::SignalStub<void, const Input::AxisEvent &> &axisEvent();
        Execution::SignalStub<void, const Input::KeyPressEvent &> &keyPressEvent();
        Execution::SignalStub<void, const Input::KeyReleaseEvent &> &keyReleaseEvent();

        virtual bool containsPoint(const Vector2 &point, const Rect2i &screenSpace, float extend = 0.0f) const;
        WidgetBase *getHoveredUp(const Vector2 &point, const Rect2i &screenSpace);
        virtual WidgetBase *getHoveredDown(const Vector2 &point, const Rect2i &screenSpace);

        virtual void render(WidgetsRenderData &renderData);

        uint16_t fetchActiveConditions(std::vector<Condition *> *conditions = nullptr);

        Geometry calculateGeometry(uint16_t activeConditions, GeometrySourceInfo *source = nullptr);

        void addConditional(uint16_t mask);
        PropertyRange conditionals();

        void setPosValue(uint16_t index, float value, uint16_t mask = 0);
        void unsetPosValue(uint16_t index, uint16_t mask);

        void setSizeValue(uint16_t index, float value, uint16_t mask = 0);
        void unsetSizeValue(uint16_t index, uint16_t mask);

        template <typename Sender>
        void addBehavior(Sender &&sender)
        {
            lifetime().attach(std::forward<Sender>(sender) | Behavior::with_named<"Widget">(Execution::ConstantBinding { this }) | Log::log_result());
        }
        Debug::DebuggableLifetime<Behavior::get_named_d> &lifetime();

        bool mVisible = true;
        std::string mName = "Unnamed";

        std::vector<Condition> mConditions;

    protected:
        Serialize::StreamResult readWidget(Serialize::CallerHierarchyFormattedSerializeStream in, std::unique_ptr<WidgetBase> &widget);
        const char *writeWidget(Serialize::CallerHierarchyFormattedSerializeStream out, const std::unique_ptr<WidgetBase> &widget) const;
        static Serialize::StreamResult scanWidget(const Serialize::SerializeTable *&out, Serialize::CallerHierarchyFormattedSerializeStream in);

        virtual void sizeChanged(const Vector3 &pixelSize);

        uint16_t fetchActiveConditionsImpl(std::vector<Condition *> &conditions);

        bool evalCondition(Condition &cond);

        virtual void updateChildrenGeometry();

    protected:
        void destroyChild(WidgetBase *w);

        Execution::Signal<void, const Input::PointerMoveEvent &> mPointerMoveSignal;
        Execution::Signal<void, const PointerClickEvent &> mPointerClickSignal;
        Execution::Signal<void, const Input::PointerMoveEvent &> mPointerEnterSignal;
        Execution::Signal<void, const Input::PointerMoveEvent &> mPointerLeaveSignal;
        Execution::Signal<void, const DragBeginEvent &> mDragBeginSignal;
        Execution::Signal<void, const DragMoveEvent &> mDragMoveSignal;
        Execution::Signal<void, const DragEndEvent &> mDragEndSignal;
        Execution::Signal<void> mDragAbortSignal;
        Execution::Signal<void, const Input::AxisEvent &> mAxisEventSignal;
        Execution::Signal<void, const Input::KeyPressEvent &> mKeyPressSignal;
        Execution::Signal<void, const Input::KeyReleaseEvent &> mKeyReleaseSignal;

        std::vector<std::unique_ptr<WidgetBase>> mChildren;

    private:
        WidgetManager &mManager;

        WidgetBase *mParent;

        Vector2 mAbsolutePos;
        Vector3 mAbsoluteSize;

        float mOpacity = 1.0f;

        bool mAcceptsPointerEvents;
        bool mAllowsDragging;

        PropertyList mProperties;

        Matrix3 mPos = Matrix3::ZERO;
        Matrix3 mSize = Matrix3::IDENTITY;

    public:
        using WidgetCreator = Serialize::ParentCreator<&WidgetBase::readWidget, &WidgetBase::writeWidget, nullptr, &WidgetBase::scanWidget>;
    };

    template <typename T>
    struct Widget : VirtualScope<T, Serialize::VirtualData<T, WidgetComponent<T, WidgetBase>>> {

        using VirtualScope<T, Serialize::VirtualData<T, WidgetComponent<T, WidgetBase>>>::VirtualScope;

        const char *getClass() const final
        {
            return T::componentName().data();
        }
    };
}
}
