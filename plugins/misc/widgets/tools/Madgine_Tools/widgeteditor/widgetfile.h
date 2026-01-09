#pragma once

#include "Madgine/render/renderpass.h"
#include "Madgine/widgets/widgetloader.h"
#include "Madgine/widgets/widgetmanager.h"

namespace Engine {
namespace Tools {

    struct WidgetFile {

        WidgetFile(WidgetEditor &editor, Widgets::WidgetLoader::Resource *resource);
        ~WidgetFile();

        void save(const Filesystem::Path &path);

        void renderSelection();
        void renderHierarchy(Widgets::WidgetBase **hoveredWidget = nullptr);
        bool drawWidget(Widgets::WidgetBase *w, Widgets::WidgetBase **hoveredWidget = nullptr);

        bool render();

    private:
        WidgetEditor &mEditor;
        Filesystem::Path mPath;
        bool mIsDirty = false;

        Widgets::WidgetManager mWidgetManager;
        Widgets::WidgetBase *mTopLevel = nullptr;

        WidgetSettings *mSelected = nullptr;
        std::map<Widgets::WidgetBase *, WidgetSettings> mSettings;

        std::unique_ptr<Render::RenderTarget> mRenderTarget;

        bool mMouseDown = false;
        bool mDragging = false;
        bool mDraggingLeft = false, mDraggingTop = false, mDraggingRight = false, mDraggingBottom = false;
    };

}
}