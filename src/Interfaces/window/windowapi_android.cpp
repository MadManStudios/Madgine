#include "../interfaceslib.h"

#if ANDROID

#    include <android/input.h>
#    include <android/native_activity.h>
#    include <android/native_window.h>

#    include "Generic/systemvariable.h"

#    include "../helpers/android_jni.h"
#    include "../input/inputevents.h"
#    include "../log/logmethods.h"
#    include "windowapi.h"
#    include "windowsettings.h"

namespace Engine {
namespace Window {

    DLL_EXPORT const PlatformCapabilities platformCapabilities {
        false,
        3.0f
    };

    SystemVariable<ANativeWindow *> sNativeWindow = nullptr;
    ANativeActivity *sActivity = nullptr;
    AInputQueue *sQueue = nullptr;

    static constexpr float sTouchMoveThreshold = 10.0f;
    static constexpr int64_t sTouchRightclickThreshold = 300000000.0;

    struct AndroidWindow;

    extern std::optional<AndroidWindow> sWindow;

    struct AndroidWindow final : OSWindow {
        AndroidWindow(ANativeWindow *window)
            : OSWindow((uintptr_t)window)
        {
        }

        bool handleMotionEvent(const AInputEvent *event)
        {
            int32_t action = AMotionEvent_getAction(event);
            size_t pointer_index = (action & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK) >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;
            InterfacesVector position {
                static_cast<int>(AMotionEvent_getX(event, pointer_index)),
                static_cast<int>(AMotionEvent_getY(event, pointer_index))
            };

            bool handled = true;

            switch (action & AMOTION_EVENT_ACTION_MASK) {
            case AMOTION_EVENT_ACTION_DOWN:
                mTouchStartPosition = position;
                mTouchStartTimestamp = AMotionEvent_getEventTime(event);
                mPendingTouch = true;
                onEvent(Input::PointerMoveEvent { position, position, position - mLastKnownMousePos });
                break;
            case AMOTION_EVENT_ACTION_UP: {
                int64_t nanoseconds = AMotionEvent_getEventTime(event) - mTouchStartTimestamp;
                Input::MouseButton::MouseButton button = nanoseconds > sTouchRightclickThreshold && mPendingTouch ? Input::MouseButton::RIGHT_BUTTON : Input::MouseButton::LEFT_BUTTON;
                if (mPendingTouch) {
                    onEvent(Input::PointerPressEvent { mTouchStartPosition, mTouchStartPosition, button });
                    mPendingTouch = false;
                }
                onEvent(Input::PointerReleaseEvent { position, position, button });
            } break;
            case AMOTION_EVENT_ACTION_MOVE:
                if (mPendingTouch && std::abs(mTouchStartPosition.x - position.x) + std::abs(mTouchStartPosition.y - position.y) > sTouchMoveThreshold) {
                    onEvent(Input::PointerPressEvent { mTouchStartPosition, mTouchStartPosition, Input::MouseButton::LEFT_BUTTON });
                    mPendingTouch = false;
                }
                onEvent(Input::PointerMoveEvent { position, position, position - mLastKnownMousePos });
                break;
            case AMOTION_EVENT_ACTION_CANCEL:
                LOG("Motion Cancel");
                break;
            case AMOTION_EVENT_ACTION_OUTSIDE:
                LOG("Motion Outside");
                break;
            case AMOTION_EVENT_ACTION_POINTER_DOWN:
                LOG("Motion Pointer Down");
                break;
            case AMOTION_EVENT_ACTION_POINTER_UP:
                LOG("Motion Pointer Up");
                break;
            case AMOTION_EVENT_ACTION_HOVER_MOVE:
                LOG("Motion Hover Move");
                break;
            case AMOTION_EVENT_ACTION_SCROLL:
                LOG("Motion Scroll");
                break;
            case AMOTION_EVENT_ACTION_HOVER_ENTER:
                LOG("Motion Hover Enter");
                break;
            case AMOTION_EVENT_ACTION_HOVER_EXIT:
                LOG("Motion Hover Exit");
                break;
            default:
                handled = false;
                LOG_ERROR("Unknown Motion Event Type: " << (action & AMOTION_EVENT_ACTION_MASK));
                break;
            }

            mLastKnownMousePos = position;
            return handled;
        }

        bool handleKeyEvent(const AInputEvent *event)
        {
            int32_t action = AKeyEvent_getAction(event);

            static const std::map<int32_t, Input::Key::Key> sKeyMap {
                { AKEYCODE_0, Input::Key::Alpha0 },
                { AKEYCODE_1, Input::Key::Alpha1 },
                { AKEYCODE_2, Input::Key::Alpha2 },
                { AKEYCODE_3, Input::Key::Alpha3 },
                { AKEYCODE_4, Input::Key::Alpha4 },
                { AKEYCODE_5, Input::Key::Alpha5 },
                { AKEYCODE_6, Input::Key::Alpha6 },
                { AKEYCODE_7, Input::Key::Alpha7 },
                { AKEYCODE_8, Input::Key::Alpha8 },
                { AKEYCODE_9, Input::Key::Alpha9 },
                { AKEYCODE_A, Input::Key::A },
                { AKEYCODE_B, Input::Key::B },
                { AKEYCODE_C, Input::Key::C },
                { AKEYCODE_D, Input::Key::D },
                { AKEYCODE_E, Input::Key::E },
                { AKEYCODE_F, Input::Key::F },
                { AKEYCODE_G, Input::Key::G },
                { AKEYCODE_H, Input::Key::H },
                { AKEYCODE_I, Input::Key::I },
                { AKEYCODE_J, Input::Key::J },
                { AKEYCODE_K, Input::Key::K },
                { AKEYCODE_L, Input::Key::L },
                { AKEYCODE_M, Input::Key::M },
                { AKEYCODE_N, Input::Key::N },
                { AKEYCODE_O, Input::Key::O },
                { AKEYCODE_P, Input::Key::P },
                { AKEYCODE_Q, Input::Key::Q },
                { AKEYCODE_R, Input::Key::R },
                { AKEYCODE_S, Input::Key::S },
                { AKEYCODE_T, Input::Key::T },
                { AKEYCODE_U, Input::Key::U },
                { AKEYCODE_V, Input::Key::V },
                { AKEYCODE_W, Input::Key::W },
                { AKEYCODE_X, Input::Key::X },
                { AKEYCODE_Y, Input::Key::Y },
                { AKEYCODE_Z, Input::Key::Z },
                { AKEYCODE_COMMA, Input::Key::Comma },
                { AKEYCODE_PERIOD, Input::Key::Period },
                { AKEYCODE_ALT_LEFT, Input::Key::LAlt },
                { AKEYCODE_ALT_RIGHT, Input::Key::RAlt },
                { AKEYCODE_SHIFT_LEFT, Input::Key::LShift },
                { AKEYCODE_SHIFT_RIGHT, Input::Key::RShift },
                { AKEYCODE_TAB, Input::Key::Tabulator },
                { AKEYCODE_SPACE, Input::Key::Space },
                { AKEYCODE_ENTER, Input::Key::Return },
                { AKEYCODE_DEL, Input::Key::Backspace },
                { AKEYCODE_MINUS, Input::Key::Minus },
                { AKEYCODE_PLUS, Input::Key::Plus },
                { AKEYCODE_PAGE_UP, Input::Key::PageUp },
                { AKEYCODE_PAGE_DOWN, Input::Key::PageDown },
                { AKEYCODE_BUTTON_A, Input::Key::GP_A },
                { AKEYCODE_BUTTON_B, Input::Key::GP_B },
                { AKEYCODE_BUTTON_X, Input::Key::GP_X },
                { AKEYCODE_BUTTON_Y, Input::Key::GP_Y },
                { AKEYCODE_BUTTON_L1, Input::Key::GP_LB },
                { AKEYCODE_BUTTON_R1, Input::Key::GP_RB },
                { AKEYCODE_BUTTON_L2, Input::Key::GP_LSB },
                { AKEYCODE_BUTTON_R2, Input::Key::GP_RSB },
                { AKEYCODE_BUTTON_START, Input::Key::GP_B1 },
                { AKEYCODE_BUTTON_SELECT, Input::Key::GP_B2 },
                { AKEYCODE_ESCAPE, Input::Key::Escape },
                { AKEYCODE_FORWARD_DEL, Input::Key::Delete },
                { AKEYCODE_CTRL_LEFT, Input::Key::LControl },
                { AKEYCODE_CTRL_RIGHT, Input::Key::RControl },
                { AKEYCODE_CAPS_LOCK, Input::Key::Capslock },
                { AKEYCODE_SCROLL_LOCK, Input::Key::ScrollLock },
                { AKEYCODE_META_LEFT, Input::Key::LWin },
                { AKEYCODE_META_RIGHT, Input::Key::RWin },
                { AKEYCODE_MOVE_HOME, Input::Key::Home },
                { AKEYCODE_MOVE_END, Input::Key::End },
                { AKEYCODE_INSERT, Input::Key::Insert },
                { AKEYCODE_F1, Input::Key::F1 },
                { AKEYCODE_F2, Input::Key::F2 },
                { AKEYCODE_F3, Input::Key::F3 },
                { AKEYCODE_F4, Input::Key::F4 },
                { AKEYCODE_F5, Input::Key::F5 },
                { AKEYCODE_F6, Input::Key::F6 },
                { AKEYCODE_F7, Input::Key::F7 },
                { AKEYCODE_F8, Input::Key::F8 },
                { AKEYCODE_F9, Input::Key::F9 },
                { AKEYCODE_F10, Input::Key::F10 },
                { AKEYCODE_F11, Input::Key::F11 },
                { AKEYCODE_F12, Input::Key::F12 },
                { AKEYCODE_NUM_LOCK, Input::Key::NumLock },
                { AKEYCODE_NUMPAD_0, Input::Key::Num0 },
                { AKEYCODE_NUMPAD_1, Input::Key::Num1 },
                { AKEYCODE_NUMPAD_2, Input::Key::Num2 },
                { AKEYCODE_NUMPAD_3, Input::Key::Num3 },
                { AKEYCODE_NUMPAD_4, Input::Key::Num4 },
                { AKEYCODE_NUMPAD_5, Input::Key::Num5 },
                { AKEYCODE_NUMPAD_6, Input::Key::Num6 },
                { AKEYCODE_NUMPAD_7, Input::Key::Num7 },
                { AKEYCODE_NUMPAD_8, Input::Key::Num8 },
                { AKEYCODE_NUMPAD_9, Input::Key::Num9 },
                { AKEYCODE_NUMPAD_DIVIDE, Input::Key::NumDivide },
                { AKEYCODE_NUMPAD_MULTIPLY, Input::Key::NumMulitply },
                { AKEYCODE_NUMPAD_SUBTRACT, Input::Key::NumSubtract },
                { AKEYCODE_NUMPAD_ADD, Input::Key::NumAdd },
                { AKEYCODE_NUMPAD_DOT, Input::Key::NumSeparator },
                { AKEYCODE_NUMPAD_COMMA, Input::Key::NumSeparator },
                { AKEYCODE_NUMPAD_ENTER, Input::Key::Return }
            };

            auto it = sKeyMap.find(AKeyEvent_getKeyCode(event));
            if (it == sKeyMap.end()) {
                LOG_ERROR("Unknown KeyCode: " << AKeyEvent_getKeyCode(event));
                return false;
            }
            Input::Key::Key key = it->second;
            jobject jevent = JNI::construct("android/view/KeyEvent", AInputEvent_getType(event), AKeyEvent_getKeyCode(event));
            char text = 0;
            if (JNI::callMemberFunction2(jevent, "isPrintingKey")) {
                text = (char)JNI::callMemberFunction3(jevent, "getUnicodeChar", AKeyEvent_getMetaState(event));
            }

            bool handled = true;

            switch (action) {
            case AKEY_EVENT_ACTION_DOWN:
                onEvent(Input::KeyPressEvent { key, text });
                break;
            case AKEY_EVENT_ACTION_UP:
                onEvent(Input::KeyReleaseEvent { key, text });
                break;
            case AKEY_EVENT_ACTION_MULTIPLE:
                LOG("Multiple Keys");
                break;
            default:
                handled = false;
                LOG_ERROR("Unknown Key Event Type: " << action);
                break;
            }

            return handled;
        }

        void onNativeWindowCreated(ANativeWindow *window)
        {
            assert(!sNativeWindow);
            sNativeWindow = window;
            if (this)
                mHandle = (uintptr_t)window;
        }

        void onNativeWindowDestroyed(ANativeWindow *window)
        {
            assert(sNativeWindow == window);
            sNativeWindow = nullptr;
            sWindow.reset();
        }

        void onNativeWindowResized(ANativeWindow *window)
        {
            mResizeNeeded.test_and_set();
        }

        void onConfigurationChanged()
        {
            mResizeNeeded.test_and_set();
        }

        void onContentRectChanged(const ARect *rect)
        {
            mContentPos = {
                rect->left,
                rect->top
            };
            mContentSize = {
                rect->right - rect->left,
                rect->bottom - rect->top
            };
        }

        void onPause()
        {
            mMinimized = true;
        }

        void onResume()
        {
            mMinimized = false;
        }

        void onInputQueueCreated(AInputQueue *queue)
        {
            assert(!sQueue);
            sQueue = queue;
        }

        void onInputQueueDestroyed(AInputQueue *queue)
        {
            assert(sQueue == queue);
            sQueue = nullptr;
        }

        // Input
        InterfacesVector mLastKnownMousePos;

        InterfacesVector mTouchStartScreenPosition;
        InterfacesVector mTouchStartPosition;
        bool mPendingTouch = false;
        int64_t mTouchStartTimestamp;

        std::atomic_flag mResizeNeeded;
        bool mMinimized = false;

        InterfacesVector mContentPos;
        InterfacesVector mContentSize;
    };

    std::optional<AndroidWindow> sWindow;

    template <auto f, typename... Args>
    static void delegate(ANativeActivity *activity, Args... args)
    {
        (*sWindow.*f)(args...);
    }

    void setup(ANativeActivity *activity)
    {
        sActivity = activity;
        activity->callbacks->onNativeWindowCreated = delegate<&AndroidWindow::onNativeWindowCreated, ANativeWindow *>;
        activity->callbacks->onNativeWindowDestroyed = delegate<&AndroidWindow::onNativeWindowDestroyed, ANativeWindow *>;
        activity->callbacks->onNativeWindowResized = delegate<&AndroidWindow::onNativeWindowResized, ANativeWindow *>;
        activity->callbacks->onConfigurationChanged = delegate<&AndroidWindow::onConfigurationChanged>;
        activity->callbacks->onContentRectChanged = delegate<&AndroidWindow::onContentRectChanged, const ARect *>;
        activity->callbacks->onPause = delegate<&AndroidWindow::onPause>;
        activity->callbacks->onResume = delegate<&AndroidWindow::onResume>;
        activity->callbacks->onInputQueueCreated = delegate<&AndroidWindow::onInputQueueCreated, AInputQueue *>;
        activity->callbacks->onInputQueueDestroyed = delegate<&AndroidWindow::onInputQueueDestroyed, AInputQueue *>;
    }

    void forceResize()
    {
        sWindow->mResizeNeeded.test_and_set();
    }

    void OSWindow::updateImpl()
    {
        if (static_cast<AndroidWindow *>(this)->mResizeNeeded.test() && sWindow->mHandle != 0) {
            static_cast<AndroidWindow *>(this)->mResizeNeeded.clear();
            onEvent(ResizeEvent { renderSize() });
        }
        if (sQueue) {
            AInputEvent *event = NULL;
            while (AInputQueue_getEvent(sQueue, &event) >= 0) {
                if (AInputQueue_preDispatchEvent(sQueue, event)) {
                    continue;
                }
                bool handled = false;
                switch (AInputEvent_getType(event)) {
                case AINPUT_EVENT_TYPE_KEY:
                    handled = static_cast<AndroidWindow *>(this)->handleKeyEvent(event);
                    break;
                case AINPUT_EVENT_TYPE_MOTION:
                    handled = static_cast<AndroidWindow *>(this)->handleMotionEvent(event);
                    break;
                default:
                    LOG_ERROR("Unknown Event Type: " << AInputEvent_getType(event));
                    break;
                }
                AInputQueue_finishEvent(sQueue, event, handled);
            }
        }
    }

    InterfacesVector OSWindow::size()
    {
        ANativeWindow *window = reinterpret_cast<ANativeWindow *>(mHandle);
        return {
            ANativeWindow_getWidth(window),
            ANativeWindow_getHeight(window)
        };
    }

    InterfacesVector OSWindow::renderSize()
    {
        return static_cast<AndroidWindow *>(this)->mContentSize;
    }

    InterfacesVector OSWindow::pos()
    {
        return { 0, 0 };
    }

    InterfacesVector OSWindow::renderPos()
    {
        return static_cast<AndroidWindow *>(this)->mContentPos;
    }

    void OSWindow::setSize(const InterfacesVector &size)
    {
        // static_cast<AndroidWindow *>(this)->mSize = size;
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
        return static_cast<AndroidWindow *>(this)->mMinimized;
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
        // sWindow.reset();
        throw 0;
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

    WindowData OSWindow::data()
    {
        return {};
    }

    // Input
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

    void OSWindow::requestSoftwareKeyboard()
    {
        ANativeActivity_showSoftInput(sActivity, ANATIVEACTIVITY_SHOW_SOFT_INPUT_IMPLICIT);
    }

    void OSWindow::releaseSoftwareKeyboard()
    {
        ANativeActivity_hideSoftInput(sActivity, ANATIVEACTIVITY_HIDE_SOFT_INPUT_IMPLICIT_ONLY);
    }

    OSWindow *sCreateWindow(const WindowSettings &settings)
    {
        sNativeWindow.wait();

        assert(!sWindow);
        sWindow.emplace(sNativeWindow);

        return &*sWindow;
    }

    std::vector<MonitorInfo> listMonitors()
    {
        int width = ANativeWindow_getWidth(sNativeWindow);
        int height = ANativeWindow_getHeight(sNativeWindow);

        MonitorInfo info { { 0, 0 }, { width, height } };

        return { info };
    }
}
}

#endif
