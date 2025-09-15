#pragma once

#include "Madgine_Tools/toolscollector.h"

#include "widgetsettings.h"

#include "Madgine_Tools/toolbase.h"

#include "widgetfile.h"

namespace Engine {
namespace Tools {

    struct WidgetEditor : public Tool<WidgetEditor> {

        SERIALIZABLEUNIT(WidgetEditor)

        WidgetEditor(ImRoot &root);

        virtual Threading::Task<bool> init() override;
        virtual Threading::Task<void> finalize() override;

        virtual void render() override;
        virtual void renderMenu() override;
        virtual void update() override;

        std::string_view key() const override;

    private:
        void renderSelection(Widgets::WidgetBase *hoveredWidget = nullptr);
        void renderHierarchy(Widgets::WidgetBase **hoveredWidget = nullptr);
        bool drawWidget(Widgets::WidgetBase *w, Widgets::WidgetBase **hoveredWidget = nullptr);

    private:        
        Widgets::WidgetManager *mWidgetManager = nullptr;
        WidgetSettings *mSelected = nullptr;
        std::map<Widgets::WidgetBase *, WidgetSettings> mSettings;

        std::vector<WidgetFile> mFiles;

        bool mMouseDown = false;
        bool mDragging = false;
        bool mDraggingLeft = false, mDraggingTop = false, mDraggingRight = false, mDraggingBottom = false;
    };

}
}