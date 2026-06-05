#include "../widgetslib.h"

#include "widget.h"

#include "Platform/window/windowapi.h"

#include "Madgine/behavior/behavior.h"

#include "Meta/reflect/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

#include "geometry.h"
#include "widgetmanager.h"

METATABLE_BEGIN(Engine::Widgets::WidgetBase)
    READONLY_PROPERTY(Children, children)
    MEMBER(mName)
    READONLY_PROPERTY(Pos, getPos)
    READONLY_PROPERTY(Size, getSize)
    MEMBER(mOpacity)
    READONLY_PROPERTY(PointerEnter, pointerEnterSender)
    READONLY_PROPERTY(PointerLeave, pointerLeaveSender)
    MEMBER(mVisible)
    MEMBER(mConditions)
METATABLE_END(Engine::Widgets::WidgetBase)

SERIALIZETABLE_BEGIN(Engine::Widgets::WidgetBase)
    FIELD(mChildren, Engine::Widgets::WidgetBase::WidgetCreator)
    FIELD(mName)
    FIELD(mPos)
    FIELD(mSize)
    FIELD(mOpacity)
    FIELD(mConditions)
    FIELD(mProperties)
SERIALIZETABLE_END(Engine::Widgets::WidgetBase)

namespace Engine {

namespace Widgets {

    WidgetBase::WidgetBase(WidgetManager &manager, WidgetBase *parent, const WidgetConfig &config)
        : mVisible(parent)
        , mPointerMoveSignal("\xef\x84\x89"
                             " Pointer Move")
        , mPointerClickSignal("\xef\x84\x89"
                              " Click")
        , mPointerEnterSignal("\xef\x84\x89"
                              " Pointer Enter")
        , mPointerLeaveSignal("\xef\x84\x89"
                              " Pointer Leave")
        , mDragBeginSignal("\xef\x84\x89"
                           " Drag Begin")
        , mDragMoveSignal("\xef\x84\x89"
                          " Drag Move")
        , mDragEndSignal("\xef\x84\x89"
                         " Drag End")
        , mDragAbortSignal("\xef\x84\x89"
                           " Drag Abort")
        , mAxisEventSignal("\xef\x84\x89"
                           " Axis Event")
        , mKeyPressSignal("\xef\x84\x89"
                          " Key Event")
        , mKeyReleaseSignal("\xef\x84\x89"
                            " Key Release")
        , mManager(manager)
        , mParent(parent)
        , mAcceptsPointerEvents(config.acceptsPointerEvents || !parent)
        , mAllowsDragging(config.allowsDragging)
    {
        mManager.registerWidget(this);

        applyGeometry(parent ? parent->getAbsoluteSize() : Math::Vector3 { Math::Vector2 { manager.mClientSpace.mSize }, Platform::Window::platformCapabilities.mScalingFactor });
    }

    WidgetBase::~WidgetBase()
    {
        mManager.unregisterWidget(this);
    }

    void WidgetBase::setSize(const Math::Matrix3 &size)
    {
        mSize = size;
        applyGeometry();
    }

    const Math::Matrix3 &WidgetBase::getSize()
    {
        return mSize;
    }

    void WidgetBase::setPos(const Math::Matrix3 &pos)
    {
        mPos = pos;
        applyGeometry();
    }

    const Math::Matrix3 &WidgetBase::getPos() const
    {
        return mPos;
    }

    void WidgetBase::setOpacity(float opacity)
    {
        mOpacity = opacity;
    }

    float WidgetBase::opacity() const
    {
        return mOpacity;
    }

    Math::Vector3 WidgetBase::getAbsoluteSize() const
    {
        return mAbsoluteSize;
    }

    Math::Vector2 WidgetBase::getAbsolutePosition() const
    {
        return mAbsolutePos;
    }

    void WidgetBase::setAbsoluteSize(const Math::Vector3 &size)
    {
        mAbsoluteSize = size;

        sizeChanged(size);

        updateChildrenGeometry();
    }

    void WidgetBase::setAbsolutePosition(const Math::Vector2 &pos)
    {
        mAbsolutePos = pos;
    }

    void WidgetBase::applyGeometry()
    {
        if (mParent)
            applyGeometry(mParent->getAbsoluteSize(), mParent->getAbsolutePosition());
        else
            applyGeometry(Math::Vector3 { Math::Vector2 { manager().getClientSpace().mSize }, 1.0f });
    }

    void WidgetBase::applyGeometry(const Math::Vector3 &parentSize, const Math::Vector2 &parentPos)
    {
        Geometry geometry = getGeometry();

        setAbsolutePosition((geometry.mPos * parentSize).xy() + parentPos);
        setAbsoluteSize(geometry.mSize * parentSize);
    }

    Geometry WidgetBase::getGeometry()
    {
        return calculateGeometry(fetchActiveConditions());
    }

    void WidgetBase::updateChildrenGeometry()
    {
        for (WidgetBase *child : children()) {
            child->applyGeometry(mAbsoluteSize, mAbsolutePos);
        }
    }

    void applyProperties(Geometry &geometry, PropertyRange range, uint16_t activeConditions, uint16_t rangeMask, GeometrySourceInfo *source)
    {

        for (auto it = range.begin(); it != range.end(); ++it) {
            const PropertyDescriptor &desc = *it;
            switch (desc.mType) {
            case PropertyType::POSITION:
                geometry.mPos[desc.mAnnotator1 / 3][desc.mAnnotator1 % 3] = it.value(0);
                if (source)
                    source->mPos[desc.mAnnotator1] = rangeMask;
                break;
            case PropertyType::SIZE:
                geometry.mSize[desc.mAnnotator1 / 3][desc.mAnnotator1 % 3] = it.value(0);
                if (source)
                    source->mSize[desc.mAnnotator1] = rangeMask;
                break;
            case PropertyType::CONDITIONAL:
                assert(rangeMask == 0);
                if ((activeConditions & desc.mAnnotator1) == desc.mAnnotator1)
                    applyProperties(geometry, it.conditionalRange(), activeConditions, desc.mAnnotator1, source);
                break;
            }
        }
    }

    Geometry WidgetBase::calculateGeometry(uint16_t activeConditions, GeometrySourceInfo *source)
    {
        Geometry geometry;
        geometry.mPos = getPos();
        geometry.mSize = getSize();

        applyProperties(geometry, mProperties, activeConditions, 0, source);

        return geometry;
    }

    const char *WidgetBase::getClass() const
    {
        return "Widget";
    }

    void WidgetBase::destroy()
    {
        if (mParent)
            mParent->destroyChild(this);
        else
            mManager.destroyTopLevel(this);
    }

    const std::string &WidgetBase::key() const
    {
        return mName;
    }

    WidgetBase *WidgetBase::createChildByDescriptor(const WidgetLoader::Handle &desc)
    {
        return mChildren.emplace_back(desc.create(mManager, this)).get();
    }

    WidgetBase *WidgetBase::createChildByAnnotation(const WidgetRegistry::Annotations &annotation)
    {
        return mChildren.emplace_back(construct(annotation, mManager, this)).get();
    }

    void WidgetBase::clearChildren()
    {
        mChildren.clear();
    }

    WidgetManager &WidgetBase::manager()
    {
        return mManager;
    }

    bool WidgetBase::dragging() const
    {
        return mManager.dragging(this);
    }

    void WidgetBase::abortDrag()
    {
        mManager.abortDrag(this);
    }

    void WidgetBase::destroyChild(WidgetBase *w)
    {
        auto it = std::ranges::find(mChildren, w, projectionToRawPtr);
        assert(it != mChildren.end());
        mChildren.erase(it);
    }

    void WidgetBase::show()
    {
        mVisible = true;
    }

    void WidgetBase::hide()
    {
        mVisible = false;
    }

    void WidgetBase::setVisible(bool v)
    {
        mVisible = v;
    }

    bool WidgetBase::isFocused() const
    {
        return mManager.focusedWidget() == this;
    }

    WidgetBase *WidgetBase::getChildRecursive(std::string_view name)
    {
        if (name == mName || name.empty())
            return this;
        for (const std::unique_ptr<WidgetBase> &w : mChildren)
            if (WidgetBase *f = w->getChildRecursive(name))
                return f;
        return nullptr;
    }

    void WidgetBase::setParent(WidgetBase *parent)
    {
        if (mParent != parent) {
            auto it = std::ranges::find(mParent->mChildren, this, projectionToRawPtr);
            parent->mChildren.emplace_back(std::move(*it));
            mParent->mChildren.erase(it);
            mParent = parent;
            applyGeometry(parent->getAbsoluteSize(), parent->getAbsolutePosition());
        }
    }

    WidgetBase *WidgetBase::getParent() const
    {
        return mParent;
    }

    void WidgetBase::setAcceptsPointerEvents(bool v)
    {
        mAcceptsPointerEvents = v;
    }

    bool WidgetBase::acceptsPointerEvents() const
    {
        return mAcceptsPointerEvents;
    }

    void WidgetBase::setAllowsDragging(bool v)
    {
        mAllowsDragging = v;
    }

    bool WidgetBase::allowsDragging() const
    {
        return mAllowsDragging;
    }

    void WidgetBase::injectPointerClick(const PointerClickEvent &arg)
    {
        mPointerClickSignal.emit(arg);
    }

    void WidgetBase::injectPointerMove(const Platform::Input::PointerMoveEvent &arg)
    {
        mPointerMoveSignal.emit(arg);
    }

    void WidgetBase::injectPointerEnter(const Platform::Input::PointerMoveEvent &arg)
    {
        mPointerEnterSignal.emit(arg);
    }

    void WidgetBase::injectPointerLeave(const Platform::Input::PointerMoveEvent &arg)
    {
        mPointerLeaveSignal.emit(arg);
    }

    void WidgetBase::injectDragBegin(const DragBeginEvent &arg)
    {
        mDragBeginSignal.emit(arg);
    }

    void WidgetBase::injectDragMove(const DragMoveEvent &arg)
    {
        mDragMoveSignal.emit(arg);
    }

    void WidgetBase::injectDragEnd(const DragEndEvent &arg)
    {
        mDragEndSignal.emit(arg);
    }

    void WidgetBase::injectDragAbort()
    {
        mDragAbortSignal.emit();
    }

    bool WidgetBase::injectAxisEvent(const Platform::Input::AxisEvent &arg)
    {
        mAxisEventSignal.emit(arg);
        return true;
    }

    bool WidgetBase::injectKeyPress(const Platform::Input::KeyPressEvent &arg)
    {
        mKeyPressSignal.emit(arg);
        return true;
    }

    bool WidgetBase::injectKeyRelease(const Platform::Input::KeyReleaseEvent &arg)
    {
        mKeyReleaseSignal.emit(arg);
        return true;
    }

    void WidgetBase::onFocusLost()
    {
    }

    Execution::SignalStub<void, const Platform::Input::PointerMoveEvent &> &WidgetBase::pointerMoveEvent()
    {
        return mPointerMoveSignal;
    }

    Execution::SignalStub<void, const PointerClickEvent &> &WidgetBase::pointerClickEvent()
    {
        return mPointerClickSignal;
    }

    Execution::SignalStub<void, const Platform::Input::PointerMoveEvent &> &WidgetBase::pointerEnterEvent()
    {
        return mPointerEnterSignal;
    }

    Execution::SignalStub<void, const Platform::Input::PointerMoveEvent &> &WidgetBase::pointerLeaveEvent()
    {
        return mPointerLeaveSignal;
    }

    Execution::SignalStub<void, const DragBeginEvent &> &WidgetBase::dragBeginEvent()
    {
        return mDragBeginSignal;
    }

    Execution::SignalStub<void, const DragMoveEvent &> &WidgetBase::dragMoveEvent()
    {
        return mDragMoveSignal;
    }

    Execution::SignalStub<void, const DragEndEvent &> &WidgetBase::dragEndEvent()
    {
        return mDragEndSignal;
    }

    Execution::SignalStub<void> &WidgetBase::dragAbortEvent()
    {
        return mDragAbortSignal;
    }

    Execution::SignalStub<void, const Platform::Input::AxisEvent &> &WidgetBase::axisEvent()
    {
        return mAxisEventSignal;
    }

    Execution::SignalStub<void, const Platform::Input::KeyPressEvent &> &WidgetBase::keyPressEvent()
    {
        return mKeyPressSignal;
    }

    Execution::SignalStub<void, const Platform::Input::KeyReleaseEvent &> &WidgetBase::keyReleaseEvent()
    {
        return mKeyReleaseSignal;
    }

    bool WidgetBase::containsPoint(const Math::Vector2 &point, const Math::Rect2i &screenSpace, float extend) const
    {
        Math::Vector2 min = mAbsolutePos + Math::Vector2 { screenSpace.mTopLeft } - extend;
        Math::Vector2 max = mAbsoluteSize.xy() + min + 2 * extend;
        return min.x <= point.x && min.y <= point.y && max.x >= point.x && max.y >= point.y;
    }

    WidgetBase *WidgetBase::getHoveredUp(const Math::Vector2 &point, const Math::Rect2i &screenSpace)
    {
        if (mVisible && containsPoint(point, screenSpace)) {
            return this;
        } else if (mParent) {
            return mParent->getHoveredUp(point, screenSpace);
        } else {
            return nullptr;
        }
    }

    WidgetBase *WidgetBase::getHoveredDown(const Math::Vector2 &point, const Math::Rect2i &screenSpace)
    {
        if (!mVisible || !containsPoint(point, screenSpace))
            return nullptr;

        for (WidgetBase *w : mChildren | std::views::transform(projectionUniquePtrToPtr)) {
            if (WidgetBase *hovered = w->getHoveredDown(point, screenSpace)) {
                return hovered;
            }
        }

        return this;
    }

    void WidgetBase::render(WidgetsRenderData &renderData)
    {
        float oldAlpha = renderData.alpha();
        size_t oldLayer = renderData.layer();

        for (const std::unique_ptr<WidgetBase> &c : mChildren) {
            if (c->mVisible) {
                renderData.setAlpha(oldAlpha * c->opacity());
                renderData.setLayer(oldLayer + 1);
                c->render(renderData);
            }
        }

        renderData.setAlpha(oldAlpha);
        renderData.setLayer(oldLayer);
    }

    Debug::DebuggableLifetime<Behavior::get_named_d> &WidgetBase::lifetime()
    {
        return mManager.lifetime();
    }

    Serialize::StreamResult WidgetBase::readWidget(Serialize::CallerHierarchyFormattedSerializeStream in, std::unique_ptr<WidgetBase> &widget)
    {
        return mManager.readWidget(in, widget, this);
    }

    const char *WidgetBase::writeWidget(Serialize::CallerHierarchyFormattedSerializeStream out, const std::unique_ptr<WidgetBase> &widget) const
    {
        return mManager.writeWidget(out, widget);
    }

    Serialize::StreamResult WidgetBase::scanWidget(const Serialize::SerializeTable *&out, Serialize::CallerHierarchyFormattedSerializeStream in)
    {
        return WidgetManager::scanWidget(out, in);
    }

    void WidgetBase::sizeChanged(const Math::Vector3 &pixelSize)
    {
    }

    uint16_t WidgetBase::fetchActiveConditions(std::vector<Condition *> *conditions)
    {
        if (!mParent)
            return 0;
        if (conditions) {
            return mParent->fetchActiveConditionsImpl(*conditions);
        } else {
            std::vector<Condition *> conditionsDummy;
            return mParent->fetchActiveConditionsImpl(conditionsDummy);
        }
    }

    uint16_t WidgetBase::fetchActiveConditionsImpl(std::vector<Condition *> &conditions)
    {
        uint16_t acc = 0;

        if (mParent)
            acc = mParent->fetchActiveConditionsImpl(conditions);

        for (Condition &cond : mConditions) {
            acc |= evalCondition(cond) << conditions.size();
            conditions.push_back(&cond);
        }

        return acc;
    }

    bool WidgetBase::evalCondition(Condition &cond)
    {
        float formulaValue;
        switch (cond.mFormula) {
        case Formula::W:
            formulaValue = mAbsoluteSize.x;
            break;
        case Formula::H:
            formulaValue = mAbsoluteSize.y;
            break;
        case Formula::W_MINUS_H:
            formulaValue = mAbsoluteSize.x - mAbsoluteSize.y;
            break;
        case Formula::ABS_W_MINUS_H:
            formulaValue = abs(mAbsoluteSize.x - mAbsoluteSize.y);
            break;
        case Formula::W_OVER_H:
            formulaValue = mAbsoluteSize.x / mAbsoluteSize.y;
            break;
        default:
            throw 0;
        }
        switch (cond.mOperator) {
        case Operator::LESS:
            return formulaValue < cond.mReferenceValue;
        case Operator::GREATER:
            return formulaValue > cond.mReferenceValue;
        case Operator::LESS_OR_EQUAL:
            return formulaValue <= cond.mReferenceValue;
        case Operator::GREATER_OR_EQUAL:
            return formulaValue >= cond.mReferenceValue;
        default:
            throw 0;
        }
    }

    void WidgetBase::addConditional(uint16_t mask)
    {
        mProperties.set(PropertyDescriptor { PropertyType::CONDITIONAL, 0, mask });
    }

    void WidgetBase::removeConditional(uint16_t mask)
    {
        mProperties.unset(PropertyDescriptor { PropertyType::CONDITIONAL, 0, mask });
    }

    PropertyRange WidgetBase::conditionals()
    {
        return {
            std::ranges::find_if(mProperties, [](const PropertyDescriptor &prop) { return prop.mType == PropertyType::CONDITIONAL; }), mProperties.end()
        };
    }

    void WidgetBase::setPosValue(uint16_t index, float value, uint16_t mask)
    {
        if (mask == 0) {
            mPos[index / 3][index % 3] = value;
        } else {
            mProperties.setConditional(mask, { PropertyType::POSITION, 0, index }, { value });
        }
        applyGeometry();
    }

    void WidgetBase::unsetPosValue(uint16_t index, uint16_t mask)
    {
        if (mask == 0) {
            LOG_WARNING("Unsetting a pos value without conditional has no effect.");
        } else {
            mProperties.unsetConditional(mask, { PropertyType::POSITION, 0, index });
        }
        applyGeometry();
    }

    void WidgetBase::setSizeValue(uint16_t index, float value, uint16_t mask)
    {
        if (mask == 0) {
            mSize[index / 3][index % 3] = value;
        } else {
            mProperties.setConditional(mask, { PropertyType::SIZE, 0, index }, { value });
        }
        applyGeometry();
    }

    void WidgetBase::unsetSizeValue(uint16_t index, uint16_t mask)
    {
        if (mask == 0) {
            LOG_WARNING("Unsetting a size value without conditional has no effect.");
        } else {
            mProperties.unsetConditional(mask, { PropertyType::SIZE, 0, index });
        }
        applyGeometry();
    }

}
}
