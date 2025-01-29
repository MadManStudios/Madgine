#include "../widgetslib.h"

#include "tablewidget.h"

#include "util/layouts/gridlayoutrenderdata.h"

#include "Meta/math/line3.h"

#include "Meta/keyvalue/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

#include "widgetmanager.h"

METATABLE_BEGIN_BASE(Engine::Widgets::TableWidget, Engine::Widgets::WidgetBase)
NAMED_MEMBER(TextData, mTextRenderData)
NAMED_MEMBER(SelectionColors, mSelectionRenderData)
PROPERTY(RowCount, rowCount, setRowCount)
PROPERTY(ColumnCount, columnCount, setColumnCount)
PROPERTY(ColumnConfigs, columnConfigs, setColumnConfigs)
METATABLE_END(Engine::Widgets::TableWidget)

SERIALIZETABLE_INHERIT_BEGIN(Engine::Widgets::TableWidget, Engine::Widgets::WidgetBase)
FIELD(mTextRenderData)
FIELD(mRowCount)
FIELD(mSelectionRenderData)
ENCAPSULATED_FIELD(mColumnConfigs, columnConfigs, setColumnConfigs)
SERIALIZETABLE_END(Engine::Widgets::TableWidget)

namespace Engine {
namespace Widgets {

    TableWidget::TableWidget(WidgetManager &manager, WidgetBase *parent) 
        : Widget(manager, parent, { .acceptsPointerEvents = true })
    {
    }

    void TableWidget::setRowCount(uint32_t count)
    {

        if (mRowCount != count) {
            mRowCount = count;
            applyGeometry();

            mCellData.resize(count * columnCount());

            if (mSelectedRow && mSelectedRow >= mRowCount)
                setSelectedRow({});
        }
    }

    void TableWidget::setColumnCount(uint32_t count)
    {
        size_t oldColumnCount = columnCount();
        if (oldColumnCount != count) {
            mColumnConfigs.resize(count);
            applyGeometry();

            size_t rows = rowCount();

            if (count < oldColumnCount && count > 0) {
                AreaView<std::string, 2> sourceView { mCellData.data(), { oldColumnCount, rows } };
                AreaView<std::string, 2> targetView { mCellData.data(), { count, rows } };
                std::ranges::move(sourceView.subArea({ 0, 0 }, { count, rows }), targetView.begin());
            }

            mCellData.resize(rows * count);

            if (count > oldColumnCount && oldColumnCount > 0) {
                AreaView<std::string, 2> sourceView { mCellData.data(), { oldColumnCount, rows } };
                AreaView<std::string, 2> targetView { mCellData.data(), { count, rows } };
                std::move(sourceView.rbegin(), sourceView.rend(), targetView.subArea({ 0, 0 }, { oldColumnCount, rows }).rbegin());
            }
        }
    }

    uint32_t TableWidget::rowCount() const
    {
        return mRowCount;
    }

    uint32_t TableWidget::columnCount() const
    {
        return mColumnConfigs.size();
    }

    std::vector<ExplicitLayoutConfig> TableWidget::columnConfigs() const
    {
        return mColumnConfigs;
    }

    void TableWidget::setColumnConfigs(const std::vector<ExplicitLayoutConfig> &configs)
    {
        setColumnCount(configs.size());
        mColumnConfigs = configs;
        applyGeometry();
    }

    AreaView<std::string, 2> TableWidget::content()
    {
        return { mCellData.data(), { columnCount(), rowCount() } };
    }

    WidgetClass TableWidget::getClass() const
    {
        return WidgetClass::TABLEWIDGET;
    }

    void TableWidget::vertices(WidgetsRenderData &renderData, size_t layer)
    {
        if (!mTextRenderData.available())
            return;

        Vector3 pos { getAbsolutePosition(), static_cast<float>(depth(layer)) };
        Vector3 size = getAbsoluteSize();

        auto keep = renderData.pushClipRect(pos.xy(), size.xy());

        if (mNeedResize)
            sizeChanged(size);

        float fullWidth = mVerticalLayoutRenderData.fullSize();
        float fullHeight = mHorizontalLayoutRenderData.fullSize(mRowCount);

        const Atlas2::Entry *blankEntry = manager().lookUpImage("blank_white");

        AreaView<std::string, 2> view { mCellData.data(), { columnCount(), mRowCount } };

        for (size_t row = 0; row < mRowCount; ++row) {
            for (size_t col = 0; col < columnCount(); ++col) {
                auto [cellPos, cellSize] = GridLayoutRenderData::getCellContentDimensions(mHorizontalLayoutRenderData, mVerticalLayoutRenderData, row, col);

                cellSize.z = size.z;
                mTextRenderData.render(renderData, view[row][col], pos + cellPos, cellSize);
            }
            auto [rowPos, rowSize] = mHorizontalLayoutRenderData.getElementDimensions(row);

            if (blankEntry) {
                Color4 color = mSelectionRenderData.mNormalColor;

                if (mHoveredRow == row) {
                    color = mSelectionRenderData.mHighlightedColor;
                } else if (mSelectedRow == row) {
                    color = mSelectionRenderData.mSelectedColor;
                }

                renderData.renderQuadUV({ pos.x, pos.y + rowPos, pos.z + 0.1f }, { fullWidth, rowSize }, color, {}, blankEntry->mArea, { 2048, 2048 }, blankEntry->mFlipped);
            }

            if (row + 1 < mRowCount) {
                float y = rowPos + rowSize;
                Line3 line {
                    { pos.x,
                        pos.y + y,
                        pos.z + 0.4f },
                    { pos.x + fullWidth,
                        pos.y + y,
                        pos.z + 0.4f }
                };
                renderData.renderLine(line);
            }
        }

        for (size_t col = 1; col < mColumnConfigs.size(); ++col) {
            auto [x, _] = mVerticalLayoutRenderData.getElementDimensions(col);
            Line3 line {
                { pos.x + x,
                    pos.y,
                    pos.z + 0.4f },
                { pos.x + x,
                    pos.y + fullHeight,
                    pos.z + 0.4f }
            };
            renderData.renderLine(line);
        }
    }

    void TableWidget::sizeChanged(const Vector3 &pixelSize)
    {
        if (mTextRenderData.available()) {
            float lineHeight = mTextRenderData.calculateLineHeight(pixelSize.z);
            SizeConstraints rowHeightConstraints {
                lineHeight + 0.1f,
                lineHeight + 6.0f,
                lineHeight + 12.0f
            };
            mHorizontalLayoutRenderData.update(mRowCount, rowHeightConstraints, pixelSize.y);
            mNeedResize = false;
        } else {
            mNeedResize = true;
        }
        mVerticalLayoutRenderData.update(mColumnConfigs, pixelSize.x, pixelSize.z);
    }

    Execution::SignalStub<IndexType<uint32_t>> &TableWidget::selectedRowChanged()
    {
        return mSelectedRowChanged;
    }

    IndexType<uint32_t> TableWidget::selectedRow() const
    {
        return mSelectedRow;
    }

    void TableWidget::setSelectedRow(IndexType<uint32_t> row)
    {
        if (mSelectedRow != row) {
            mSelectedRow = row;
            mSelectedRowChanged.emit(mSelectedRow);
        }
    }

    IndexType<uint32_t> TableWidget::rowIndex(float y)
    {
        uint32_t index = static_cast<uint32_t>(y / mHorizontalLayoutRenderData.getElementDimensions(0).second);
        if (index >= mRowCount)
            return {};
        return index;
    }

    void TableWidget::injectPointerEnter(const Input::PointerEventArgs &arg)
    {
        mHoveredRow = rowIndex(arg.windowPosition.y);
        WidgetBase::injectPointerEnter(arg);
    }

    void TableWidget::injectPointerLeave(const Input::PointerEventArgs &arg)
    {
        mHoveredRow.reset();
        WidgetBase::injectPointerLeave(arg);
    }

    void TableWidget::injectPointerMove(const Input::PointerEventArgs &arg)
    {
        mHoveredRow = rowIndex(arg.windowPosition.y);
        WidgetBase::injectPointerMove(arg);
    }

    void TableWidget::injectPointerClick(const Input::PointerEventArgs &arg)
    {
        setSelectedRow(mHoveredRow);
        WidgetBase::injectPointerClick(arg);
    }

}
}
