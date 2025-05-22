#include "../interfaceslib.h"

#if EMSCRIPTEN

#    include "windowapi.h"
#    include "windowsettings.h"

#    include <EGL/egl.h>

#    include <emscripten/html5.h>

#    include "Generic/systemvariable.h"

#    include "../input/inputevents.h"

namespace Engine {
namespace Window {

    DLL_EXPORT const PlatformCapabilities platformCapabilities {
        false,
        1.0f
    };

    // DLL_EXPORT Threading::SystemVariable<ANativeWindow*> sNativeWindow = nullptr;

    DLL_EXPORT EGLDisplay sDisplay = EGL_NO_DISPLAY;

    static struct DisplayGuard {
        DisplayGuard()
        {
            sDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
            if (sDisplay != EGL_NO_DISPLAY) {
                if (!eglInitialize(sDisplay, nullptr, nullptr))
                    sDisplay = EGL_NO_DISPLAY;
            }
        }

        ~DisplayGuard()
        {
            if (sDisplay != EGL_NO_DISPLAY)
                eglTerminate(sDisplay);
        }
    } sDisplayGuard;

    struct EmscriptenWindow final : OSWindow {

        template <auto f, typename... Args>
        static auto delegate(Args... args, void *userData)
        {
            return (static_cast<EmscriptenWindow *>(userData)->*f)(args...);
        }

        EmscriptenWindow(EGLSurface surface, WindowEventListener *listener)
            : OSWindow((uintptr_t)surface, listener)
            , mKeyDown {}
        {
            EGLint width;
            EGLint height;
            if (!eglQuerySurface(sDisplay, surface, EGL_WIDTH, &width) || !eglQuerySurface(sDisplay, surface, EGL_HEIGHT, &height))
                std::terminate();
            mSize = { width, height };

            static constexpr auto MouseDelegate = delegate<&EmscriptenWindow::handleMouseEvent, int, const EmscriptenMouseEvent *>;
            static constexpr auto WheelDelegate = delegate<&EmscriptenWindow::handleWheelEvent, int, const EmscriptenWheelEvent *>;
            static constexpr auto KeyDelegate = delegate<&EmscriptenWindow::handleKeyEvent, int, const EmscriptenKeyboardEvent *>;
            static constexpr auto TouchDelegate = delegate<&EmscriptenWindow::handleTouchEvent, int, const EmscriptenTouchEvent *>;

            // Input
            emscripten_set_mousemove_callback("#canvas", this, 0, MouseDelegate);

            emscripten_set_mousedown_callback("#canvas", this, 0, MouseDelegate);
            emscripten_set_mouseup_callback(EMSCRIPTEN_EVENT_TARGET_DOCUMENT, this, 0, MouseDelegate);
            emscripten_set_wheel_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, this, 0, WheelDelegate);

            emscripten_set_keydown_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, this, 0, KeyDelegate);
            emscripten_set_keyup_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, this, 0, KeyDelegate);

            emscripten_set_touchmove_callback("#canvas", this, 0, TouchDelegate);

            emscripten_set_touchstart_callback("#canvas", this, 0, TouchDelegate);
            emscripten_set_touchend_callback(EMSCRIPTEN_EVENT_TARGET_DOCUMENT, this, 0, TouchDelegate);
        }

        static Input::MouseButton::MouseButton convertMouseButton(unsigned short id)
        {
            switch (id) {
            case 0:
                return Input::MouseButton::LEFT_BUTTON;
            case 1:
                return Input::MouseButton::MIDDLE_BUTTON;
            case 2:
                return Input::MouseButton::RIGHT_BUTTON;
            default:
                std::terminate();
            }
        }

        EM_BOOL handleMouseEvent(int eventType, const EmscriptenMouseEvent *mouseEvent)
        {
            InterfacesVector position = { mouseEvent->targetX, mouseEvent->targetY };
            InterfacesVector screenPosition = { mouseEvent->screenX, mouseEvent->screenY };

            bool handled = false;

            switch (eventType) {
            case EMSCRIPTEN_EVENT_MOUSEMOVE:                
                handled = injectPointerMove({ position, screenPosition,
                    { mouseEvent->movementX, mouseEvent->movementY } });
                break;
            case EMSCRIPTEN_EVENT_MOUSEDOWN:
                handled = injectPointerPress({ position, screenPosition,
                    convertMouseButton(mouseEvent->button) });
                break;
            case EMSCRIPTEN_EVENT_MOUSEUP:
                handled = injectPointerRelease({ position, screenPosition,
                    convertMouseButton(mouseEvent->button) });
                break;
            }

            mLastMousePosition = position;

            return handled;
        }

        EM_BOOL handleKeyEvent(int eventType, const EmscriptenKeyboardEvent *keyEvent)
        {

            mKeyDown[Input::Key::Shift] = keyEvent->shiftKey;
            mKeyDown[Input::Key::Control] = keyEvent->ctrlKey;
            mKeyDown[Input::Key::Alt] = keyEvent->altKey;

            switch (eventType) {
            case EMSCRIPTEN_EVENT_KEYDOWN:
                mKeyDown[keyEvent->keyCode] = true;
                char text;
                switch (keyEvent->keyCode) {
                case Input::Key::Shift:
                case Input::Key::Control:
                case Input::Key::Alt:
                    text = 0;
                    break;
                default:
                    text = keyEvent->key[0];
                }
                return injectKeyPress({ static_cast<Input::Key::Key>(keyEvent->keyCode), text, controlKeyState() });
            case EMSCRIPTEN_EVENT_KEYUP:
                mKeyDown[keyEvent->keyCode] = false;
                return injectKeyRelease({ static_cast<Input::Key::Key>(keyEvent->keyCode), 0, controlKeyState() });
            }

            return EM_FALSE;
        }

        EM_BOOL handleWheelEvent(int eventType, const EmscriptenWheelEvent *wheelEvent)
        {

            switch (eventType) {
            case EMSCRIPTEN_EVENT_WHEEL:
                return injectAxisEvent(Input::AxisEventArgs { Input::AxisEventArgs::WHEEL, static_cast<float>(wheelEvent->deltaY) });
            }

            return EM_FALSE;
        }

        EM_BOOL handleTouchEvent(int eventType, const EmscriptenTouchEvent *touchEvent)
        {
            InterfacesVector position = { touchEvent->touches[0].targetX, touchEvent->touches[0].targetY };
            InterfacesVector screenPosition = { touchEvent->touches[0].screenX, touchEvent->touches[0].screenY };

            bool handled = false;

            switch (eventType) {
            case EMSCRIPTEN_EVENT_TOUCHMOVE:
                handled = injectPointerMove({ position, screenPosition, position - mLastMousePosition });
                break;
            case EMSCRIPTEN_EVENT_TOUCHSTART:
                injectPointerMove({ position, screenPosition, position - mLastMousePosition });
                handled = injectPointerPress({ position, screenPosition, Input::MouseButton::LEFT_BUTTON });
                break;
            case EMSCRIPTEN_EVENT_TOUCHEND:
                handled = injectPointerRelease({ position, screenPosition, Input::MouseButton::LEFT_BUTTON });
                break;
            }

            mLastMousePosition = position;

            return handled;
        }

        Input::ControlKeyState controlKeyState() const
        {
            return {
                mKeyDown[Input::Key::Shift],
                mKeyDown[Input::Key::Control],
                mKeyDown[Input::Key::Alt]
            };
        }

        InterfacesVector mSize;
        InterfacesVector mLastMousePosition;

        // Input
        bool mKeyDown[512];
    };

    static std::unordered_map<EGLSurface, EmscriptenWindow> sWindows;

    void OSWindow::update()
    {
    }

    InterfacesVector OSWindow::size()
    {
        return static_cast<EmscriptenWindow *>(this)->mSize;
    }

    InterfacesVector OSWindow::renderSize()
    {
        // TODO
        return size();
    }

    InterfacesVector OSWindow::pos()
    {
        return { 0, 0 };
    }

    InterfacesVector OSWindow::renderPos()
    {
        return { 0, 0 };
    }

    void OSWindow::setSize(const InterfacesVector &size)
    {
        static_cast<EmscriptenWindow *>(this)->mSize = size;
        emscripten_set_canvas_element_size("#canvas", size.x, size.y);
    }

    void OSWindow::setRenderSize(const InterfacesVector &size)
    {
        setSize(size);
    }

    void OSWindow::setPos(const InterfacesVector &pos)
    {
    }

    void OSWindow::setRenderPos(const InterfacesVector &pos)
    {
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
        return true;
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
        return "emscripten-window";
    }

    void OSWindow::close()
    {
    }

    void OSWindow::destroy()
    {
        eglDestroySurface(sDisplay, (EGLSurface)mHandle);
        sWindows.erase((EGLSurface)mHandle);
    }

    // Input
    bool OSWindow::isKeyDown(Input::Key::Key key)
    {
        return static_cast<EmscriptenWindow *>(this)->mKeyDown[key];
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

    EM_BOOL eventCallback(int type, const EmscriptenUiEvent *event, void *userData)
    {
        double w;
        double h;

        emscripten_get_element_css_size("#canvas", &w, &h);

        static_cast<EmscriptenWindow *>(userData)->setSize({ static_cast<int>(w), static_cast<int>(h) });
        return true;
    }

    OSWindow *sCreateWindow(const WindowSettings &settings, WindowEventListener *listener)
    {
        assert(sDisplay);

        EGLSurface handle = (EGLSurface)settings.mHandle;
        if (!handle) {

            const EGLint attribs[] = {
                EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
                EGL_BLUE_SIZE, 8,
                EGL_GREEN_SIZE, 8,
                EGL_RED_SIZE, 8,
                EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
                EGL_CONFORMANT, EGL_OPENGL_ES2_BIT,
                EGL_NONE
            };

            EGLConfig config;
            EGLint numConfigs;
            EGLint format;

            if (!eglChooseConfig(sDisplay, attribs, &config, 1, &numConfigs))
                return nullptr;

            if (!eglGetConfigAttrib(sDisplay, config, EGL_NATIVE_VISUAL_ID, &format))
                return nullptr;

            handle = eglCreateWindowSurface(sDisplay, config, 0, 0);
            if (!handle)
                return nullptr;
        }

        auto pib = sWindows.try_emplace(handle, handle, listener);
        assert(pib.second);

        EmscriptenWindow *window = &pib.first->second;

        emscripten_set_resize_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, window, false, &eventCallback);

        double w;
        double h;

        emscripten_get_element_css_size("#canvas", &w, &h);
        window->setSize({ static_cast<int>(w), static_cast<int>(h) });

        return window;
    }

    std::vector<MonitorInfo> listMonitors()
    {
        double w;
        double h;

        emscripten_get_element_css_size("#canvas", &w, &h);

        MonitorInfo info { 0, 0, static_cast<int>(w), static_cast<int>(h) };

        return { info };
    }

    WindowData OSWindow::data()
    {
        return {};
    }
}
}

#endif
