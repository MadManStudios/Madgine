#include "../widgetstoolslib.h"

#include "widgeteditor.h"
#include "widgetfile.h"

#include "Madgine_Tools/imgui/clientimroot.h"

#include "Madgine/window/mainwindow.h"

#include "Madgine/render/rendercontext.h"

#include "Madgine/widgets/widgetmanager.h"

#include "Madgine/widgets/util/widgetsrenderdata.h"

#include "Madgine/render/rendertarget.h"

#include "Madgine/widgets/widget.h"

namespace Engine {
namespace Tools {

    WidgetFile::WidgetFile(WidgetEditor &editor, Widgets::WidgetLoader::Handle handle)
        : mEditor(editor)
        , mPath(handle.info()->resource()->path())
        , mRenderTarget(static_cast<ClientImRoot &>(editor.root()).window().getRenderer()->createRenderTexture({ 1, 1 }, { .mFormat = Render::FORMAT_RGBA8_SRGB, .mName { handle.info()->resource()->name() } }))
        , mHandle(std::move(handle))
    {
        static_cast<ClientImRoot &>(editor.root()).addRenderTarget(mRenderTarget.get());
        mRenderTarget->addRenderPass(this);

    }

    WidgetFile::~WidgetFile()
    {
        mRenderTarget->removeRenderPass(this);
        static_cast<ClientImRoot &>(mEditor.root()).removeRenderTarget(mRenderTarget.get());
    }

    

    void WidgetFile::save(const Filesystem::Path &path)
    {
    }

    void WidgetFile::render(Render::RenderTarget *target, size_t iteration)
    {
        if (!mWidget) {
            if (mHandle.available()) {
                mWidget = mHandle.create(mEditor.manager());
                mWidget->applyGeometry(Vector3 { Vector2 { mRenderTarget->size() }, 1.0f });
            }
            return;
        }

        Widgets::WidgetsRenderData renderData;

        renderData.setAlpha(mWidget->opacity());
        renderData.setLayer(0);
        mWidget->render(renderData);

        mEditor.manager().render(target, renderData, mRenderTarget->size());
    }

    int WidgetFile::priority() const
    {
        return 50;
    }

    std::string_view WidgetFile::name() const
    {
        return "Widget File";
    }

}
}
