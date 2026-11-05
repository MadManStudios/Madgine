#include "../widgetslib.h"

#include "scrollarea.h"

#include "Platform/input/inputevents.h"

#include "Meta/reflect/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

#include "geometry.h"
#include "util/layouts/sizeconstraints.h"

NAMED_UNIQUECOMPONENT(ScrollArea, Engine::Widgets::ScrollArea);

METATABLE_BEGIN_BASE(Engine::Widgets::ScrollArea, Engine::Widgets::WidgetBase)
METATABLE_END(Engine::Widgets::ScrollArea)

SERIALIZETABLE_INHERIT_BEGIN(Engine::Widgets::ScrollArea, Engine::Widgets::WidgetBase)
SERIALIZETABLE_END(Engine::Widgets::ScrollArea)

namespace Engine {
namespace Widgets {

    struct LayoutAxisElement {
        SizeConstraints mConstraints;
        float mOffset;
    };

    bool ScrollArea::appliesLayout() const
    {
        return true;
    }

    void ScrollArea::updateChildrenGeometry()
    {

        std::vector<LayoutAxisElement> rows;
        std::vector<LayoutAxisElement> cols;

        for (WidgetBase *child : children()) {
            LayoutPos pos = child->layoutPos();
            LayoutSize size = child->layoutSize();

            int col = pos.mColumn;
            int row = pos.mRow;

            if (cols.size() < col + pos.mColumnSpan) {
                cols.resize(col + pos.mColumnSpan);
            }
            if (rows.size() < row + pos.mRowSpan) {
                rows.resize(row + pos.mRowSpan);
            }

            for (size_t i = 0; i < pos.mColumnSpan; ++i)
                cols[col + i].mConstraints = max(cols[col + i].mConstraints, size.mConstraints.mWidth / pos.mColumnSpan);
            for (size_t i = 0; i < pos.mRowSpan; ++i)
                rows[row + i].mConstraints = max(rows[row + i].mConstraints, size.mConstraints.mHeight / pos.mRowSpan);
        }

        SizeConstraints width;
        SizeConstraints height;

        for (const LayoutAxisElement &col : cols) {
            width += col.mConstraints;
        }
        for (const LayoutAxisElement &row : rows) {
            height += row.mConstraints;
        }

        float scrollBarSize = 20.0f;

        Math::Vector3 availableSize = getAbsoluteSize();

        float rowFactor = height.calculateFactor(availableSize.y);
        if (rowFactor < 0.0f) {
            rowFactor = 0.0f;
            availableSize.x -= scrollBarSize;

            mMaxScroll = height.apply(0.0f) - availableSize.y;
        } else {
            mMaxScroll = 0.0f;
        }
        mScroll = std::min(mMaxScroll, mScroll);

        float colFactor = width.calculateFactor(availableSize.x);

        float offset = 0.0f;
        for (LayoutAxisElement &col : cols) {
            col.mOffset = offset;
            offset += col.mConstraints.apply(colFactor);
        }
        offset = 0.0f;
        for (LayoutAxisElement &row : rows) {
            row.mOffset = offset;
            offset += row.mConstraints.apply(rowFactor);
        }

        for (WidgetBase *child : children()) {

            Geometry geometry = child->getGeometry();

            LayoutPos pos = child->layoutPos();
            LayoutSize size = child->layoutSize();

            if (pos.mColumnSpan == 0 || pos.mRowSpan == 0)
                continue;

            float width = 0.0f;
            float height = 0.0f;
            for (size_t i = 0; i < pos.mColumnSpan; ++i)
                width += cols[pos.mColumn + i].mConstraints.apply(colFactor);
            for (size_t i = 0; i < pos.mRowSpan; ++i)
                height += rows[pos.mRow + i].mConstraints.apply(rowFactor);

            Math::Vector2 absolutePosition = getClientSpacePosition() + Math::Vector2 { cols[pos.mColumn].mOffset, rows[pos.mRow].mOffset };
            if (size.mConstraints.mWidth.mMax < width) {
                absolutePosition.x += pos.mXAlign * (width - size.mConstraints.mWidth.mMax);
            }
            if (size.mConstraints.mHeight.mMax < height) {
                absolutePosition.y += pos.mYAlign * (height - size.mConstraints.mHeight.mMax);
            }
            Math::Vector3 absoluteSize = {
                std::min(width, size.mConstraints.mWidth.mMax),
                std::min(height, size.mConstraints.mHeight.mMax),
                getAbsoluteSize().z
            };

            absolutePosition.y -= mScroll;

            child->setClientSpacePosition(absolutePosition);
            child->setAbsoluteSize(absoluteSize);
        }
    }

    bool ScrollArea::injectAxisEvent(const Platform::Input::AxisEvent &arg)
    {
        bool consumed = false;
        if (arg.mAxisType == Platform::Input::AxisEvent::WHEEL) {
            float diff = -50.0f * arg.mAxis1;
            float oldScroll = mScroll;
            mScroll = Math::clamp(oldScroll + diff, 0.0f, mMaxScroll);
            consumed = mScroll - oldScroll == diff;
            updateChildrenGeometry();
        }
        WidgetBase::injectAxisEvent(arg);
        return consumed;
    }

}
}
