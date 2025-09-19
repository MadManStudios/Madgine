#pragma once

#include "Madgine_Tools/toolscollector.h"

#include "widgetsettings.h"

#include "Madgine_Tools/toolbase.h"

#include "widgetfile.h"

#include "Madgine_Tools/resourceeditor.h"

#include "Madgine/widgets/widgetloader.h"

namespace Engine {
namespace Tools {

    struct WidgetEditor : public Tool<WidgetEditor, ResourceEditor> {

        SERIALIZABLEUNIT(WidgetEditor)

        WidgetEditor(ImRoot &root);

        Threading::Task<bool> init() override;
        Threading::Task<void> finalize() override;

        void render() override;
        void renderMenu() override;
        void update() override;

        std::string_view key() const override;

        void open(Resources::ResourceBase *res) override;

        Widgets::WidgetManager &manager();

    private:
        void renderSelection(Widgets::WidgetBase *hoveredWidget = nullptr);
        void renderHierarchy(Widgets::WidgetBase **hoveredWidget = nullptr);
        bool drawWidget(Widgets::WidgetBase *w, Widgets::WidgetBase **hoveredWidget = nullptr);

    private:        
        Widgets::WidgetManager *mWidgetManager = nullptr;
        WidgetSettings *mSelected = nullptr;
        std::map<Widgets::WidgetBase *, WidgetSettings> mSettings;        

        std::map<Widgets::WidgetLoader::Resource *, WidgetFile> mFiles;

        bool mMouseDown = false;
        bool mDragging = false;
        bool mDraggingLeft = false, mDraggingTop = false, mDraggingRight = false, mDraggingBottom = false;
    };

}
}