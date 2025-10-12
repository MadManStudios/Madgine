#pragma once

#include "Interfaces/window/windoweventlistener.h"

namespace Engine {
namespace Window {

    struct MADGINE_CLIENT_EXPORT ToolWindow : WindowEventListener {    
        ToolWindow(MainWindow &parent, const WindowSettings &settings);
        virtual ~ToolWindow();

        void close();

        OSWindow *osWindow();

        Render::RenderTarget *getRenderer();

    protected:
        bool onWindowEvent(const WindowEvent &event) override;

    private:
        MainWindow &mParent;
        OSWindow *mOsWindow = nullptr;
        std::unique_ptr<Render::RenderTarget> mRenderWindow;

    };

}
}
