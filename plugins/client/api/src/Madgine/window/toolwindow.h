#pragma once

namespace Engine {
namespace Window {

    struct MADGINE_CLIENT_EXPORT ToolWindow {
        ToolWindow(MainWindow &parent, const WindowSettings &settings);
        virtual ~ToolWindow();

        void close();

        OSWindow *osWindow();

        Render::RenderTarget *getRenderer();

        bool onWindowEvent(const WindowEvent &event);

    private:
        MainWindow &mParent;
        OSWindow *mOsWindow = nullptr;
        std::unique_ptr<Render::RenderTarget> mRenderWindow;
    };

}
}
