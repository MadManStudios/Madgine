#pragma once

#include "widget.h"

namespace Engine {
namespace Widgets {
    struct MADGINE_WIDGETS_EXPORT SceneWindow : Widget<SceneWindow> {        
        SceneWindow(WidgetManager &manager, WidgetBase *parent = nullptr);

        virtual ~SceneWindow();

        void render(WidgetsRenderData &renderData) override;        

        void setRenderSource(Render::RenderTarget *source);

        std::string getClass() const override;

    protected:
        void sizeChanged(const Vector3 &pixelSize) override;

    private:
        Render::RenderTarget *mSource = nullptr;
    };
}
}
