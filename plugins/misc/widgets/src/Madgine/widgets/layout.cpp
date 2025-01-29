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

            if (cols.size() <= col) {
                cols.resize(col + 1);
            }
            if (rows.size() <= row) {
                rows.resize(row + 1);
            }

            cols[col].mConstraints = max(cols[col].mConstraints, size.mConstraints.mWidth);
            rows[row].mConstraints = max(rows[row].mConstraints, size.mConstraints.mHeight);
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

            LayoutAxisElement &col = cols[pos.mColumn];
            LayoutAxisElement &row = rows[pos.mRow];

            Vector2 absolutePosition = getAbsolutePosition() + Vector2 { col.mOffset, row.mOffset };
            Vector3 absoluteSize = {
                std::min(col.mConstraints.apply(colFactor), size.mConstraints.mWidth.mMax),
                std::min(row.mConstraints.apply(rowFactor), size.mConstraints.mHeight.mMax),
                getAbsoluteSize().z
            };

            child->setAbsolutePosition(absolutePosition);
            child->setAbsoluteSize(absoluteSize);            
        }
    }

}
}
