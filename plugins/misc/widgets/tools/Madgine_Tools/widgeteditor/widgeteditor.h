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
        
        void renderMenu() override;
        void render() override;

        std::string_view key() const override;

        void open(Resources::ResourceBase *res) override;

        Widgets::WidgetManager &manager();

        static void renderWidgetBorders(Widgets::WidgetBase *widget, Engine::Vector2i screenOffset, ImU32 color);

    private:

        Widgets::WidgetBase *handleManagerInteractions(Widgets::WidgetManager &manager, const ImVec2 &pos);

    private:        
        Widgets::WidgetManager *mWidgetManager = nullptr;
        Inspector *mInspector = nullptr;

        friend struct WidgetFile;
        std::map<Widgets::WidgetLoader::Resource *, WidgetFile> mFiles;

        bool mHierarchyVisible = true;
        bool mWidgetDetailsVisible = true;
    };

}
}