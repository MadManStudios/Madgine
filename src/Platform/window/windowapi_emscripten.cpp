#include "../platformlib.h"

#if EMSCRIPTEN

#    include <EGL/egl.h>
#    include <emscripten/html5.h>

#    include "Generic/systemvariable.h"

#    include "../input/inputevents.h"
#    include "windowapi.h"
#    include "windowsettings.h"

namespace Engine {
namespace Platform {
    namespace Window {

        DLL_EXPORT const PlatformCapabilities platformCapabilities {
            false,
            1.0f
        };

        // DLL_EXPORT Threading::SystemVariable<ANativeWindow*> sNativeWindow = nullptr;

        DLL_EXPORT EGLDisplay sDisplay = EGL_NO_DISPLAY;

        static constexpr float sTouchMoveThreshold = 10.0f;
        static constexpr double sTouchRightclickThreshold = 300.0;

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

            EmscriptenWindow(EGLSurface surface)
                : OSWindow((uintptr_t)surface)
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
                emscripten_set_mouseup_callback("#canvas", this, 0, MouseDelegate);
                emscripten_set_wheel_callback("#canvas", this, 0, WheelDelegate);

                emscripten_set_keydown_callback("#canvas", this, 0, KeyDelegate);
                emscripten_set_keyup_callback("#canvas", this, 0, KeyDelegate);

                emscripten_set_touchmove_callback("#canvas", this, 0, TouchDelegate);

                emscripten_set_touchstart_callback("#canvas", this, 0, TouchDelegate);
                emscripten_set_touchend_callback("#canvas", this, 0, TouchDelegate);
            }

            void focus()
            {
                if (mSoftwareKeyboardRequested)
                    EM_ASM(
                        Module.input.focus(););
                else
                    EM_ASM(
                        Module.canvas.focus(););
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
                PlatformVector position = { mouseEvent->targetX, mouseEvent->targetY };
                PlatformVector screenPosition = { mouseEvent->screenX, mouseEvent->screenY };

                bool handled = true;

                switch (eventType) {
                case EMSCRIPTEN_EVENT_MOUSEMOVE:
                    onEvent(Input::PointerMoveEvent { position, screenPosition,
                        { mouseEvent->movementX, mouseEvent->movementY } });
                    break;
                case EMSCRIPTEN_EVENT_MOUSEDOWN:
                    focus();
                    onEvent(Input::PointerPressEvent { position, screenPosition,
                        convertMouseButton(mouseEvent->button) });
                    break;
                case EMSCRIPTEN_EVENT_MOUSEUP:
                    onEvent(Input::PointerReleaseEvent { position, screenPosition,
                        convertMouseButton(mouseEvent->button) });
                    break;
                default:
                    handled = false;
                }

                mLastMousePosition = position;

                return handled;
            }

            EM_BOOL handleKeyEvent(int eventType, const EmscriptenKeyboardEvent *keyEvent)
            {
                using namespace Input::Key;

                static std::map<std::string, Input::Key::Key> sKeys {
                    { "Backspace", Backspace },
                    { "Tab", Tabulator },
                    { "Delete", Delete },
                    { "Enter", Return },
                    { "Pause", Pause },
                    { "CapsLock", Capslock },
                    { "Escape", Escape },
                    { "PageUp", PageUp },
                    { "PageDown", PageDown },
                    { "End", End },
                    { "Home", Home },
                    { "ArrowLeft", LeftArrow },
                    { "ArrowUp", UpArrow },
                    { "ArrowRight", RightArrow },
                    { "ArrowDown", DownArrow },
                    { "PrintScreen", PrintScreen },
                    { "Insert", Insert },
                    { "Delete", Delete },
                    { "Help", Help },
                    { "MetaLeft", LWin },
                    { "MetaRight", RWin },
                    { "NumLock", NumLock },
                    { "ScrollLock", ScrollLock },
                    { "ShiftLeft", LShift },
                    { "ShiftRight", RShift },
                    { "ControlLeft", LControl },
                    { "ControlRight", RControl },
                    { "AltLeft", LAlt },
                    { "AltRight", RAlt },

                    // Alpha
                    { "Space", Space },
                    { "KeyA", A },
                    { "KeyB", B },
                    { "KeyC", C },
                    { "KeyD", D },
                    { "KeyE", E },
                    { "KeyF", F },
                    { "KeyG", G },
                    { "KeyH", H },
                    { "KeyI", I },
                    { "KeyJ", J },
                    { "KeyK", K },
                    { "KeyL", L },
                    { "KeyM", M },
                    { "KeyN", N },
                    { "KeyO", O },
                    { "KeyP", P },
                    { "KeyQ", Q },
                    { "KeyR", R },
                    { "KeyS", S },
                    { "KeyT", T },
                    { "KeyU", U },
                    { "KeyV", V },
                    { "KeyW", W },
                    { "KeyX", X },
                    { "KeyY", Y },
                    { "KeyZ", Z },

                    { "Semicolon", OEM1 }, // US-Layout: ;:
                    { "?", Plus },
                    { "Comma", Comma },
                    { "Minus", Minus },
                    { "Period", Period },
                    { "Slash", OEM2 }, // US-Layout: /?
                    { "Backquote", OEM3 }, // US-Layout: `~
                    { "BracketLeft", OEM4 }, // US-Layout: [{
                    { "Backslash", OEM5 }, // US-Layout: \|
                    { "BracketRight", OEM6 }, // US-Layout: ]}
                    { "Quote", OEM7 }, // US-Layout: '"

                    // Num
                    { "Digit0", Alpha0 },
                    { "Digit1", Alpha1 },
                    { "Digit2", Alpha2 },
                    { "Digit3", Alpha3 },
                    { "Digit4", Alpha4 },
                    { "Digit5", Alpha5 },
                    { "Digit6", Alpha6 },
                    { "Digit7", Alpha7 },
                    { "Digit8", Alpha8 },
                    { "Digit9", Alpha9 },

                    { "Numpad0", Num0 },
                    { "Numpad1", Num1 },
                    { "Numpad2", Num2 },
                    { "Numpad3", Num3 },
                    { "Numpad4", Num4 },
                    { "Numpad5", Num5 },
                    { "Numpad6", Num6 },
                    { "Numpad7", Num7 },
                    { "Numpad8", Num8 },
                    { "Numpad9", Num9 },

                    // Special
                    { "NumpadMultiply", NumMulitply },
                    { "NumpadAdd", NumAdd },
                    { "NumpadComma", NumSeparator },
                    { "NumpadSubtract", NumSubtract },
                    { "NumpadDecimal", NumDecimal },
                    { "NumpadDivide", NumDivide },

                    { "F1", F1 },
                    { "F2", F2 },
                    { "F3", F3 },
                    { "F4", F4 },
                    { "F5", F5 },
                    { "F6", F6 },
                    { "F7", F7 },
                    { "F8", F8 },
                    { "F9", F9 },
                    { "F10", F10 },
                    { "F11", F11 },
                    { "F12", F12 },
                };

                auto it = keyEvent->code ? sKeys.find(keyEvent->code) : sKeys.end();
                if (it == sKeys.end()) {
                    LOG_ERROR("Unknown key event! code: " << keyEvent->code << ", key: " << keyEvent->key << ", charCode: " << keyEvent->charCode << ", keyCode: " << keyEvent->keyCode << ", which: " << keyEvent->which);
                    return EM_FALSE;
                }
                Input::Key::Key key = it->second;

                mKeyDown[Input::Key::Shift] = keyEvent->shiftKey;
                mKeyDown[Input::Key::Control] = keyEvent->ctrlKey;
                mKeyDown[Input::Key::Alt] = keyEvent->altKey;

                char text = 0;

                bool handled = true;

                switch (eventType) {
                case EMSCRIPTEN_EVENT_KEYDOWN:
                    mKeyDown[key] = true;
                    if (keyEvent->key[1] == '\0')
                        text = keyEvent->key[0];
                    onEvent(Input::KeyPressEvent { key, text, controlKeyState() });
                    break;
                case EMSCRIPTEN_EVENT_KEYUP:
                    mKeyDown[key] = false;
                    onEvent(Input::KeyReleaseEvent { key, text, controlKeyState() });
                    break;
                default:
                    handled = false;
                }

                return handled;
            }

            EM_BOOL handleWheelEvent(int eventType, const EmscriptenWheelEvent *wheelEvent)
            {

                bool handled = true;

                switch (eventType) {
                case EMSCRIPTEN_EVENT_WHEEL: {
                    float delta = wheelEvent->deltaY;
                    switch (wheelEvent->deltaMode) {
                    case DOM_DELTA_PIXEL:
                        break;
                    case DOM_DELTA_LINE:
                        delta *= 32.0f;
                        break;
                    case DOM_DELTA_PAGE:
                        delta *= 1068.0f;
                    }
                    onEvent(Input::AxisEvent { Input::AxisEvent::WHEEL, -delta / 120.0f });
                } break;
                default:
                    handled = false;
                }

                return handled;
            }

            EM_BOOL handleTouchEvent(int eventType, const EmscriptenTouchEvent *touchEvent)
            {
                PlatformVector position = { touchEvent->touches[0].targetX, touchEvent->touches[0].targetY };
                PlatformVector screenPosition = { touchEvent->touches[0].screenX, touchEvent->touches[0].screenY };

                bool handled = true;

                switch (eventType) {
                case EMSCRIPTEN_EVENT_TOUCHMOVE:
                    if (mPendingTouch && std::abs(mTouchStartPosition.x - position.x) + std::abs(mTouchStartPosition.y - position.y) > sTouchMoveThreshold) {
                        onEvent(Input::PointerPressEvent { mTouchStartPosition, mTouchStartScreenPosition, Input::MouseButton::LEFT_BUTTON });
                        mPendingTouch = false;
                    }
                    onEvent(Input::PointerMoveEvent { position, screenPosition, position - mLastMousePosition });
                    break;
                case EMSCRIPTEN_EVENT_TOUCHSTART:
                    focus();
                    mTouchStartScreenPosition = screenPosition;
                    mTouchStartPosition = position;
                    mTouchStartTimestamp = touchEvent->timestamp;
                    mPendingTouch = true;
                    onEvent(Input::PointerMoveEvent { position, screenPosition, position - mLastMousePosition });
                    break;
                case EMSCRIPTEN_EVENT_TOUCHEND: {
                    double milliseconds = touchEvent->timestamp - mTouchStartTimestamp;
                    Input::MouseButton::MouseButton button = milliseconds > sTouchRightclickThreshold && mPendingTouch ? Input::MouseButton::RIGHT_BUTTON : Input::MouseButton::LEFT_BUTTON;
                    if (mPendingTouch) {
                        onEvent(Input::PointerPressEvent { mTouchStartPosition, mTouchStartScreenPosition, button });
                        mPendingTouch = false;
                    }
                    onEvent(Input::PointerReleaseEvent { position, screenPosition, button });
                } break;
                default:
                    handled = false;
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

            static EM_BOOL eventCallback(int type, const EmscriptenUiEvent *event, void *userData)
            {
                double w;
                double h;

                emscripten_get_element_css_size("#canvas", &w, &h);

                EmscriptenWindow *window = static_cast<EmscriptenWindow *>(userData);

                window->setSize({ static_cast<int>(w), static_cast<int>(h) });

                window->onEvent(ResizeEvent { static_cast<int>(w), static_cast<int>(h) });

                return true;
            }

            PlatformVector mSize;
            PlatformVector mLastMousePosition;

            PlatformVector mTouchStartScreenPosition;
            PlatformVector mTouchStartPosition;
            bool mPendingTouch = false;
            double mTouchStartTimestamp;

            // Input
            bool mKeyDown[512];

            bool mSoftwareKeyboardRequested = false;
        };

        static std::unordered_map<EGLSurface, EmscriptenWindow> sWindows;

        void OSWindow::updateImpl()
        {
        }

        PlatformVector OSWindow::size()
        {
            return static_cast<EmscriptenWindow *>(this)->mSize;
        }

        PlatformVector OSWindow::renderSize()
        {
            // TODO
            return size();
        }

        PlatformVector OSWindow::pos()
        {
            return { 0, 0 };
        }

        PlatformVector OSWindow::renderPos()
        {
            return { 0, 0 };
        }

        void OSWindow::setSize(const PlatformVector &size)
        {
            static_cast<EmscriptenWindow *>(this)->mSize = size;
            emscripten_set_canvas_element_size("#canvas", size.x, size.y);
        }

        void OSWindow::setRenderSize(const PlatformVector &size)
        {
            setSize(size);
        }

        void OSWindow::setPos(const PlatformVector &pos)
        {
        }

        void OSWindow::setRenderPos(const PlatformVector &pos)
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

        void OSWindow::requestSoftwareKeyboard()
        {
            static_cast<EmscriptenWindow *>(this)->mSoftwareKeyboardRequested = true;
            EM_ASM(
                if (document.activeElement == Module.canvas) {
                    Module.input.focus();
                });
        }

        void OSWindow::releaseSoftwareKeyboard()
        {
            static_cast<EmscriptenWindow *>(this)->mSoftwareKeyboardRequested = false;
            EM_ASM(
                if (document.activeElement == Module.input) {
                    Module.canvas.focus();
                });
        }

        void OSWindow::setCursorIcon(Input::CursorIcon icon)
        {
            switch (icon) {
            case Input::CursorIcon::Arrow:
                EM_ASM(document.body.style.cursor = 'default';);
                break;
            case Input::CursorIcon::TextInput:
                EM_ASM(document.body.style.cursor = 'text';);
                break;
            case Input::CursorIcon::ResizeAll:
                EM_ASM(document.body.style.cursor = 'move';);
                break;
            case Input::CursorIcon::ResizeNS:
                EM_ASM(document.body.style.cursor = 'ns-resize';);
                break;
            case Input::CursorIcon::ResizeEW:
                EM_ASM(document.body.style.cursor = 'ew-resize';);
                break;
            case Input::CursorIcon::ResizeNESW:
                EM_ASM(document.body.style.cursor = 'nesw-resize';);
                break;
            case Input::CursorIcon::ResizeNWSE:
                EM_ASM(document.body.style.cursor = 'nwse-resize';);
                break;
            case Input::CursorIcon::Hand:
                EM_ASM(document.body.style.cursor = 'grab';);
                break;
            case Input::CursorIcon::NotAllowed:
                EM_ASM(document.body.style.cursor = 'not-allowed';);
                break;
            default:
                LOG_ERROR("Unhandled cursor icon: " << (int)icon);
            }
        }

        std::string OSWindow::getClipboardString()
        {
            return "";
        }

        bool OSWindow::setClipboardString(std::string_view s)
        {
            return true;
        }

        OSWindow *sCreateWindow(const WindowSettings &settings)
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

            auto pib = sWindows.try_emplace(handle, handle);
            assert(pib.second);

            EmscriptenWindow *window = &pib.first->second;

            emscripten_set_resize_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, window, false, &EmscriptenWindow::eventCallback);

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

            MonitorInfo info { { 0, 0 }, { static_cast<int>(w), static_cast<int>(h) } };

            return { info };
        }

        WindowData OSWindow::data()
        {
            return {};
        }

    }
}
}

#endif
