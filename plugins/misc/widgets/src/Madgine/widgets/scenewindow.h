#pragma once

#include "widget.h"

namespace Engine {
namespace Widgets {
    struct MADGINE_WIDGETS_EXPORT SceneWindow : Widget<SceneWindow> {        
        SceneWindow(WidgetManager &manager, WidgetBase *parent = nullptr);

        virtual ~SceneWindow();

        virtual void vertices(WidgetsRenderData &renderData, size_t layer) override;        

        void setRenderSource(Render::RenderTarget *source);

        virtual WidgetClass getClass() const override;

    protected:
        void sizeChanged(const Vector3 &pixelSize) override;

    private:
        Render::RenderTarget *mSource = nullptr;
    };
}
}
