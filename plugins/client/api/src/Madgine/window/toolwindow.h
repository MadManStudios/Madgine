#pragma once

namespace Engine {
namespace Core {

    struct MADGINE_CLIENT_EXPORT ToolWindow {
        ToolWindow(MainWindow &parent, const Platform::Window::WindowSettings &settings);
        virtual ~ToolWindow();

        void close();

        Platform::Window::OSWindow *osWindow();

        Render::RenderTarget *getRenderer();

        bool onWindowEvent(const Platform::Window::WindowEvent &event);

    private:
        MainWindow &mParent;
        Platform::Window::OSWindow *mOsWindow = nullptr;
        std::unique_ptr<Render::RenderTarget> mRenderWindow;
    };

}
}
