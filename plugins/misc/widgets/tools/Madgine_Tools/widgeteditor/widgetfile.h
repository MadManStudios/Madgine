#pragma once

#include "Madgine/widgets/widgetloader.h"

#include "Madgine/render/renderpass.h"

namespace Engine {
namespace Tools {

    struct WidgetFile : Render::RenderPass {

        WidgetFile(WidgetEditor &editor, Widgets::WidgetLoader::Handle handle);
        ~WidgetFile();

        bool render();

        void save(const Filesystem::Path &path);

        void render(Render::RenderTarget *target, size_t iteration) override;

        int priority() const override;

        std::string_view name() const override;

    private:
        WidgetEditor &mEditor;
        Filesystem::Path mPath;
        bool mIsDirty = false;

        std::unique_ptr<Render::RenderTarget> mRenderTarget;
        std::unique_ptr<Widgets::WidgetBase> mWidget;        

        Widgets::WidgetLoader::Handle mHandle;
    };

}
}