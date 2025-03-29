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

        bool injectKeyPress(const Input::KeyEventArgs &arg) override;
        bool injectKeyRelease(const Input::KeyEventArgs &arg) override;
        bool injectPointerPress(const Input::PointerEventArgs &arg) override;
        bool injectPointerRelease(const Input::PointerEventArgs &arg) override;
        bool injectPointerMove(const Input::PointerEventArgs &arg) override;

    protected:
        void onClose() override;
        void onRepaint() override;
        void onResize(const InterfacesVector &size) override;

    private:
        MainWindow &mParent;
        OSWindow *mOsWindow = nullptr;
        std::unique_ptr<Render::RenderTarget> mRenderWindow;

    };

}
}
