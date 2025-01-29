#pragma once

#include "widget.h"

#include "util/colortintrenderdata.h"
#include "util/layouts/explicitlayoutrenderdata.h"
#include "util/layouts/uniformlayoutrenderdata.h"
#include "util/textrenderdata.h"

#include "Generic/areaview.h"

namespace Engine {
namespace Widgets {
    struct MADGINE_WIDGETS_EXPORT TableWidget : Widget<TableWidget> {

        SERIALIZABLEUNIT(TableWidget);

        TableWidget(WidgetManager &manager, WidgetBase *parent = nullptr);

        virtual ~TableWidget() = default;

        void setRowCount(uint32_t count);
        void setColumnCount(uint32_t count);

        uint32_t rowCount() const;
        uint32_t columnCount() const;

        std::vector<ExplicitLayoutConfig> columnConfigs() const;
        void setColumnConfigs(const std::vector<ExplicitLayoutConfig> &configs);

        AreaView<std::string, 2> content();

        virtual WidgetClass getClass() const override;

        virtual void vertices(WidgetsRenderData &renderData, size_t layer) override;

        virtual void sizeChanged(const Vector3 &pixelSize) override;

        Execution::SignalStub<IndexType<uint32_t>> &selectedRowChanged();
        IndexType<uint32_t> selectedRow() const;
        void setSelectedRow(IndexType<uint32_t> row);

    protected:
        IndexType<uint32_t> rowIndex(float y);

        void injectPointerEnter(const Input::PointerEventArgs &arg) override;
        void injectPointerLeave(const Input::PointerEventArgs &arg) override;
        void injectPointerMove(const Input::PointerEventArgs &arg) override;

        void injectPointerClick(const Input::PointerEventArgs &arg) override;

    private:
        UniformLayoutRenderData mHorizontalLayoutRenderData;
        ExplicitLayoutRenderData mVerticalLayoutRenderData;
        TextRenderData mTextRenderData;

        ColorTintRenderData mSelectionRenderData;

        std::vector<std::string> mCellData;

        uint32_t mRowCount = 0;
        std::vector<ExplicitLayoutConfig> mColumnConfigs;

        bool mNeedResize = false;

        IndexType<uint32_t> mHoveredRow;
        IndexType<uint32_t> mSelectedRow;

        Execution::Signal<IndexType<uint32_t>> mSelectedRowChanged;
    };
}
}

REGISTER_TYPE(Engine::Widgets::TableWidget)
