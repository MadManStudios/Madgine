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
        EmscriptenWindow(EGLSurface surface, WindowEventListener *listener)
            : OSWindow((uintptr_t)surface, listener)
            , mKeyDown {}
        {
            EGLint width;
            EGLint height;
            if (!eglQuerySurface(sDisplay, surface, EGL_WIDTH, &width) || !eglQuerySurface(sDisplay, surface, EGL_HEIGHT, &height))
                std::terminate();
            mSize = { width, height };

            // Input
            emscripten_set_mousemove_callback("#canvas", this, 0, EmscriptenWindow::handleMouseEvent);

            emscripten_set_mousedown_callback("#canvas", this, 0, EmscriptenWindow::handleMouseEvent);
            emscripten_set_mouseup_callback(EMSCRIPTEN_EVENT_TARGET_DOCUMENT, this, 0, EmscriptenWindow::handleMouseEvent);
            emscripten_set_wheel_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, this, 0, EmscriptenWindow::handleWheelEvent);

            emscripten_set_keydown_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, this, 0, EmscriptenWindow::handleKeyEvent);
            emscripten_set_keyup_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, this, 0, EmscriptenWindow::handleKeyEvent);

            emscripten_set_touchstart_callback("#canvas", this, 0, EmscriptenWindow::handleTouchEvent);
            emscripten_set_touchend_callback("#canvas", this, 0, EmscriptenWindow::handleTouchEvent);
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

        static EM_BOOL handleMouseEvent(int eventType, const EmscriptenMouseEvent *mouseEvent, void *userData)
        {
            EmscriptenWindow *_this = static_cast<EmscriptenWindow *>(userData);

            switch (eventType) {
            case EMSCRIPTEN_EVENT_MOUSEMOVE:
                _this->mLastMousePosition = { mouseEvent->targetX, mouseEvent->targetY };
                return _this->injectPointerMove({ _this->mLastMousePosition, { mouseEvent->screenX, mouseEvent->screenY },
                    { mouseEvent->movementX, mouseEvent->movementY } });
            case EMSCRIPTEN_EVENT_MOUSEDOWN:
                _this->mLastMousePosition = { mouseEvent->targetX, mouseEvent->targetY };
                return _this->injectPointerPress({ _this->mLastMousePosition, { mouseEvent->screenX, mouseEvent->screenY },
                    convertMouseButton(mouseEvent->button) });
            case EMSCRIPTEN_EVENT_MOUSEUP:
                return _this->injectPointerRelease({ _this->mLastMousePosition, { mouseEvent->screenX, mouseEvent->screenY },
                    convertMouseButton(mouseEvent->button) });
            }

            return EM_FALSE;
        }

        static EM_BOOL handleKeyEvent(int eventType, const EmscriptenKeyboardEvent *keyEvent, void *userData)
        {
            EmscriptenWindow *_this = static_cast<EmscriptenWindow *>(userData);

            _this->mKeyDown[Input::Key::Shift] = keyEvent->shiftKey;
            _this->mKeyDown[Input::Key::Control] = keyEvent->ctrlKey;
            _this->mKeyDown[Input::Key::Alt] = keyEvent->altKey;

            switch (eventType) {
            case EMSCRIPTEN_EVENT_KEYDOWN:
                _this->mKeyDown[keyEvent->keyCode] = true;
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
                return _this->injectKeyPress({ static_cast<Input::Key::Key>(keyEvent->keyCode), text, _this->controlKeyState() });
            case EMSCRIPTEN_EVENT_KEYUP:
                _this->mKeyDown[keyEvent->keyCode] = false;
                return _this->injectKeyRelease({ static_cast<Input::Key::Key>(keyEvent->keyCode), 0, _this->controlKeyState() });
            }

            return EM_FALSE;
        }

        static EM_BOOL handleWheelEvent(int eventType, const EmscriptenWheelEvent *wheelEvent, void *userData)
        {
            EmscriptenWindow *_this = static_cast<EmscriptenWindow *>(userData);

            switch (eventType) {
            case EMSCRIPTEN_EVENT_WHEEL:
                return _this->injectAxisEvent(Input::AxisEventArgs { Input::AxisEventArgs::WHEEL, static_cast<float>(wheelEvent->deltaY) });            
            }

            return EM_FALSE;
        }

        static EM_BOOL handleTouchEvent(int eventType, const EmscriptenTouchEvent *touchEvent, void *userData)
        {
            EmscriptenWindow *_this = static_cast<EmscriptenWindow *>(userData);

            switch (eventType) {
            case EMSCRIPTEN_EVENT_TOUCHMOVE:
                _this->mLastMousePosition = { touchEvent->touches[0].targetX, touchEvent->touches[0].targetY };
                return _this->injectPointerMove({ _this->mLastMousePosition, { touchEvent->touches[0].screenX, touchEvent->touches[0].screenY },
                    { /* touchEvent->touches[0].movementX, touchEvent->touches[0].movementY*/0, 0 } });
            case EMSCRIPTEN_EVENT_TOUCHSTART:
                _this->mLastMousePosition = { touchEvent->touches[0].targetX, touchEvent->touches[0].targetY };
                return _this->injectPointerPress({ _this->mLastMousePosition, { touchEvent->touches[0].screenX, touchEvent->touches[0].screenY },
                    Input::MouseButton::LEFT_BUTTON });
            case EMSCRIPTEN_EVENT_TOUCHEND:
                return _this->injectPointerRelease({ _this->mLastMousePosition, { touchEvent->touches[0].screenX, touchEvent->touches[0].screenY },
                    Input::MouseButton::LEFT_BUTTON });
            }

            return EM_FALSE;
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

            // sNativeWindow.wait();

            // ANativeWindow_setBuffersGeometry(sNativeWindow, 0, 0, format);

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
