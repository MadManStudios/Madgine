#include "../widgetslib.h"

#include "layout.h"

#include "util/layouts/sizeconstraints.h"

#include "geometry.h"

#include "Meta/keyvalue/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

METATABLE_BEGIN_BASE(Engine::Widgets::Layout, Engine::Widgets::WidgetBase)
METATABLE_END(Engine::Widgets::Layout)

SERIALIZETABLE_INHERIT_BEGIN(Engine::Widgets::Layout, Engine::Widgets::WidgetBase)
SERIALIZETABLE_END(Engine::Widgets::Layout)

namespace Engine {
namespace Widgets {

    struct LayoutPos {
        float mColumn;
        float mColumnSpan;
        float mXAlign;
        float mRow;
        float mRowSpan;
        float mYAlign;
    };

    struct LayoutSize {
        SizeConstraints2 mConstraints;
        Vector3 mDefaultZ;
    };

    struct LayoutAxisElement {
        SizeConstraints mConstraints;
        float mOffset;
    };

    WidgetClass Layout::getClass() const
    {
        return WidgetClass::LAYOUT;
    }

    void Layout::updateChildrenGeometry()
    {

        std::vector<LayoutAxisElement> rows;
        std::vector<LayoutAxisElement> cols;

        for (WidgetBase *child : children()) {
            Matrix3 matrixPos = child->getPos().Transpose();
            Matrix3 matrixSize = child->getSize().Transpose();

            LayoutPos pos = reinterpret_cast<LayoutPos &>(matrixPos);
            LayoutSize size = reinterpret_cast<LayoutSize &>(matrixSize);

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

        float colFactor = width.calculateFactor(getAbsoluteSize().x);
        float rowFactor = height.calculateFactor(getAbsoluteSize().y);

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

            Geometry geometry = child->calculateGeometry(child->fetchActiveConditions());

            Matrix3 matrixPos = geometry.mPos.Transpose();
            Matrix3 matrixSize = geometry.mSize.Transpose();

            LayoutPos pos = reinterpret_cast<LayoutPos &>(matrixPos);
            LayoutSize size = reinterpret_cast<LayoutSize &>(matrixSize);

            float width = 0.0f;
            float height = 0.0f;
            for (size_t i = 0; i < pos.mColumnSpan; ++i)
                width += cols[pos.mColumn + i].mConstraints.apply(colFactor);
            for (size_t i = 0; i < pos.mRowSpan; ++i)
                height += rows[pos.mRow + i].mConstraints.apply(rowFactor);



            Vector2 absolutePosition = getAbsolutePosition() + Vector2 { 
                cols[pos.mColumn].mOffset, 
                rows[pos.mRow].mOffset
            };
            if (size.mConstraints.mWidth.mMax < width) {
                absolutePosition.x += pos.mXAlign * (width - size.mConstraints.mWidth.mMax);
            }
            if (size.mConstraints.mHeight.mMax < height){
                absolutePosition.y += pos.mYAlign * (height - size.mConstraints.mHeight.mMax);
            }
            Vector3 absoluteSize = {
                std::min(width, size.mConstraints.mWidth.mMax),
                std::min(height, size.mConstraints.mHeight.mMax),
                getAbsoluteSize().z
            };

            child->setAbsolutePosition(absolutePosition);
            child->setAbsoluteSize(absoluteSize);            
        }
    }

}
}
