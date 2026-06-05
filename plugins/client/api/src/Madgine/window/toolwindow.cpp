#include "../clientlib.h"

#include "toolwindow.h"

#include "Platform/window/windowapi.h"

#include "Madgine/render/rendercontext.h"
#include "Madgine/render/rendertarget.h"

#include "mainwindow.h"

namespace Engine {
namespace Core {
    ToolWindow::ToolWindow(MainWindow &parent, const Platform::Window::WindowSettings &settings)
        : mParent(parent)
    {
        mOsWindow = sCreateWindow(settings);

        mRenderWindow = parent.getRenderer()->createRenderWindow(mOsWindow);
    }

    ToolWindow::~ToolWindow()
    {
        mOsWindow->destroy();
    }

    void ToolWindow::close()
    {
        mParent.destroyToolWindow(this);
    }

    Platform::Window::OSWindow *ToolWindow::osWindow()
    {
        return mOsWindow;
    }

    Render::RenderTarget *ToolWindow::getRenderer()
    {
        return mRenderWindow.get();
    }

    bool ToolWindow::onWindowEvent(const Platform::Window::WindowEvent &event)
    {
        return std::visit(overloaded {
                              [this](const Platform::Window::CloseEvent &) {
                                  close();
                                  return true;
                              },
                              [](const Platform::Window::RepaintEvent &) {
                                  return true;
                              },
                              [this](const Platform::Window::ResizeEvent &e) {
                                  mRenderWindow->resize({ e.mSize.x, e.mSize.y });
                                  return true;
                              },
                              [this](const auto &event) {
                                  return mParent.onWindowEvent(event);
                              } },
            event);
    }

}
}
