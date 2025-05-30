#include "../interfaceslib.h"

#if LINUX

#    include "windowapi.h"
#    include "windowsettings.h"

#    include "../input/inputevents.h"

#    include <GL/glx.h>
#    include <X11/Xlib.h>

namespace Engine {
namespace Window {

    DLL_EXPORT const PlatformCapabilities platformCapabilities {
        true,
        1.0f
    };

    static Atom WM_DELETE_WINDOW;

    DLL_EXPORT Display *sDisplay()
    {

        static struct DisplayGuard {
            DisplayGuard()
            {
                display = XOpenDisplay(NULL);
                if (display)
                    WM_DELETE_WINDOW = XInternAtom(display, "WM_DELETE_WINDOW", False);
            }

            ~DisplayGuard()
            {
                if (display)
                    XCloseDisplay(display);
            }
            Display *display;
        } sDisplayGuard;

        return sDisplayGuard.display;
    }

    struct LinuxWindow final : OSWindow {
        LinuxWindow(::Window hwnd, WindowEventListener *listener, const InterfacesVector &size)
            : OSWindow(hwnd, listener)
            , mSize(size)
        {
        }

        static Input::MouseButton::MouseButton convertMouseButton(int button)
        {
            switch (button) {
            case 1:
                return Input::MouseButton::LEFT_BUTTON;
            case 2:
                return Input::MouseButton::MIDDLE_BUTTON;
            case 3:
                return Input::MouseButton::RIGHT_BUTTON;
            default:
                std::terminate();
            }
        }

        bool handle(const XEvent &e)
        {
            switch (e.type) {
            case MotionNotify: {
                const XMotionEvent &xme = e.xmotion;
                InterfacesVector mousePos { xme.x, xme.y };
                injectPointerMove({ mousePos, { xme.x_root, xme.y_root }, mousePos - mLastMousePosition });
                mLastMousePosition = mousePos;
                break;
            }
            case ButtonPress: {
                const XButtonEvent &xbe = e.xbutton;
                injectPointerPress({ { xbe.x, xbe.y }, { xbe.x_root, xbe.y_root }, convertMouseButton(xbe.button) });
                break;
            }
            case ButtonRelease: {
                const XButtonEvent &xbe = e.xbutton;
                injectPointerRelease({ { xbe.x, xbe.y }, { xbe.x_root, xbe.y_root }, convertMouseButton(xbe.button) });
                break;
            }
            case ConfigureNotify: {
                const XConfigureEvent &xce = e.xconfigure;

                /* This event type is generated for a variety of
                                           happenings, so check whether the window has been
                                           resized. */

                if (xce.width != mSize.x || xce.height != mSize.y) {
                    mSize = { xce.width, xce.height };
                    onResize(mSize);
                }
                break;
            }
            case ClientMessage: {
                const XClientMessageEvent &xcme = e.xclient;
                if (xcme.data.l[0] == WM_DELETE_WINDOW) {
                    onClose();
                }
                break;
            }
            case DestroyNotify:
                return false;
            }
            return true;
        }

        InterfacesVector mSize;

        InterfacesVector mLastMousePosition;
    };

    static std::unordered_map<::Window, LinuxWindow> sWindows;

    void OSWindow::update()
    {
        // TODO: correct handling of different windows
        XEvent event;
        while (XPending(sDisplay())) {
            XNextEvent(sDisplay(), &event);
            auto it = sWindows.find(event.xany.window);
            if (it != sWindows.end()) {
                if (!it->second.handle(event))
                    sWindows.erase(it);
            }
        }
    }

    InterfacesVector OSWindow::size()
    {
        return static_cast<LinuxWindow*>(this)->mSize;
    }

    InterfacesVector OSWindow::renderSize()
    {
        // TODO
        return size();
    }

    InterfacesVector OSWindow::pos()
    {
        XWindowAttributes xwa;
        XGetWindowAttributes(sDisplay(), mHandle, &xwa);
        return { xwa.x, xwa.y };
    }

    InterfacesVector OSWindow::renderPos()
    {
        XWindowAttributes xwa;
        XGetWindowAttributes(sDisplay(), mHandle, &xwa);
        return { xwa.x + xwa.border_width, xwa.y + xwa.border_width };
    }

    void OSWindow::setSize(const InterfacesVector &size)
    {
        static_cast<LinuxWindow*>(this)->mSize = size;
    }

    void OSWindow::setRenderSize(const InterfacesVector &size)
    {
        // TODO
        setSize(size);
    }

    void OSWindow::setPos(const InterfacesVector &pos)
    {
        // TODO
    }

    void OSWindow::setRenderPos(const InterfacesVector &pos)
    {
        // TODO
    }

    void OSWindow::show()
    {
    }

    bool OSWindow::isMinimized()
    {
        return false;
    }

    bool OSWindow::isMaximized()
    {
        return false;
    }

    void OSWindow::focus()
    {
    }

    bool OSWindow::hasFocus()
    {
        return true;
    }

    void OSWindow::setTitle(const char *title)
    {
    }

    std::string OSWindow::title() const
    {
        return "";
    }

    void OSWindow::close()
    {
    }

    void OSWindow::destroy()
    {
        XDestroyWindow(sDisplay(), mHandle);
    }

    WindowData OSWindow::data()
    {
        return {};
    }

    bool OSWindow::isKeyDown(Input::Key::Key key)
    {
        return false;
    }

    void OSWindow::captureInput()
    {
    }

    void OSWindow::releaseInput()
    {
    }

    void OSWindow::setCursorIcon(Input::CursorIcon icon)
    {
        /*SetCursor(LoadCursor(NULL, [](Input::CursorIcon icon) {
            switch (icon) {
            case Input::CursorIcon::Arrow:
                return IDC_ARROW;
            case Input::CursorIcon::TextInput:
                return IDC_IBEAM;
            case Input::CursorIcon::ResizeAll:
                return IDC_SIZEALL;
            case Input::CursorIcon::ResizeNS:
                return IDC_SIZENS;
            case Input::CursorIcon::ResizeEW:
                return IDC_SIZEWE;
            case Input::CursorIcon::ResizeNESW:
                return IDC_SIZENESW;
            case Input::CursorIcon::ResizeNWSE:
                return IDC_SIZENWSE;
            case Input::CursorIcon::Hand:
                return IDC_HAND;
            case Input::CursorIcon::NotAllowed:
                return IDC_NO;
            default:
                throw 0;
            }
        }(icon)));*/
    }

    std::string OSWindow::getClipboardString()
    {
        return "";
    }

    bool OSWindow::setClipboardString(std::string_view s)
    {
        return true;
    }

    OSWindow *sCreateWindow(const WindowSettings &settings, WindowEventListener *listener)
    {
        assert(sDisplay());

        uintptr_t handle = settings.mHandle;
        if (!handle) {

            static ::Window root = DefaultRootWindow(sDisplay());

            static int att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };

            static XVisualInfo *vi = glXChooseVisual(sDisplay(), 0, att);
            assert(vi);

            static Colormap cmap = XCreateColormap(sDisplay(), root, vi->visual, AllocNone);

            static XSetWindowAttributes swa;

            swa.colormap = cmap;
            swa.event_mask = ExposureMask | KeyPressMask;

            InterfacesVector pos = settings.mData.mPosition.x < 0 || settings.mData.mPosition.y < 0 ? InterfacesVector { 0, 0 } : settings.mData.mPosition;

            handle = XCreateWindow(sDisplay(), root, pos.x, pos.y, settings.mData.mSize.x, settings.mData.mSize.y, 0, vi->depth, InputOutput, vi->visual, CWColormap | CWEventMask, &swa);

            XStoreName(sDisplay(), handle, settings.mTitle);

            if (!handle)
                return nullptr;
        }

        XSelectInput(sDisplay(), handle, StructureNotifyMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask | KeyPressMask | KeyReleaseMask);

        XMapWindow(sDisplay(), handle);

        XSetWMProtocols(sDisplay(), handle, &WM_DELETE_WINDOW, 1);

        XEvent event;
        do {
            XWindowEvent(sDisplay(), handle, StructureNotifyMask, &event);
        } while (event.type != MapNotify);

        auto pib = sWindows.try_emplace(handle, handle, listener, settings.mData.mSize);
        assert(pib.second);

        return &pib.first->second;
    }

    static std::vector<MonitorInfo> sBuffer;

    static void updateMonitors()
    {
        sBuffer.clear();
        for (int i = 0; i < XScreenCount(sDisplay()); ++i) {
            Screen *screen = XScreenOfDisplay(sDisplay(), i);
            // TODO position
            sBuffer.push_back({ 0, 0, XWidthOfScreen(screen), XHeightOfScreen(screen) });
        }
    }

    std::vector<MonitorInfo> listMonitors()
    {
        if (sBuffer.empty())
            updateMonitors();
        return sBuffer;
    }

}
}

#endif
