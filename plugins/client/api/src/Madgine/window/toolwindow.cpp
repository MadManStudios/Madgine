#include "../clientlib.h"

#include "toolwindow.h"

#include "Interfaces/window/windowapi.h"

#include "Madgine/render/rendercontext.h"
#include "Madgine/render/rendertarget.h"

#include "mainwindow.h"

namespace Engine {
namespace Window {
    ToolWindow::ToolWindow(MainWindow &parent, const WindowSettings &settings)
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

    OSWindow *ToolWindow::osWindow()
    {
        return mOsWindow;
    }

    Render::RenderTarget *ToolWindow::getRenderer()
    {
        return mRenderWindow.get();
    }

    bool ToolWindow::onWindowEvent(const WindowEvent &event)
    {
        return std::visit(overloaded {
                              [this](const CloseEvent &) {
                                  close();
                                  return true;
                              },
                              [](const RepaintEvent &) {
                                  return true;
                              },
                              [this](const ResizeEvent &e) {
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
