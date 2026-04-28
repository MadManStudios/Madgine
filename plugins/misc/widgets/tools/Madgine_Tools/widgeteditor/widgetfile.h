#pragma once

#include "Madgine/render/renderpass.h"
#include "Madgine/widgets/widgetloader.h"
#include "Madgine/widgets/widgetmanager.h"

#include "Madgine_Tools/resources/resourcefile.h"

namespace Engine {
namespace Tools {

    struct WidgetFile : ResourceFile<WidgetEditor> {

        WidgetFile(WidgetEditor &editor, Widgets::WidgetLoader::Resource *resource);
        ~WidgetFile();

        void saveAs(const Filesystem::Path &path) override;

        void renderSelection();
        void renderHierarchy(Widgets::WidgetBase **hoveredWidget = nullptr);
        bool drawWidget(Widgets::WidgetBase *w, Widgets::WidgetBase **hoveredWidget = nullptr);

        void render();

        Widgets::WidgetManager &widgetManager();

    private:

        Widgets::WidgetManager mWidgetManager;
        Widgets::WidgetBase *mTopLevel = nullptr;

        WidgetSettings *mSelected = nullptr;
        std::map<Widgets::WidgetBase *, WidgetSettings> mSettings;

        std::unique_ptr<Render::RenderTarget> mRenderTarget;
        
        bool mDraggingLeft = false, mDraggingTop = false, mDraggingRight = false, mDraggingBottom = false;
    };

}
}