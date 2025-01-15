#include "../interfaceslib.h"

#if ANDROID

#    include "windowapi.h"
#    include "windowsettings.h"

#    include <android/input.h>
#    include <android/native_window.h>

#    include "Generic/systemvariable.h"

#    include "../input/inputevents.h"

#    include "../log/logmethods.h"

#    include <android/native_activity.h>

namespace Engine {
namespace Window {

    DLL_EXPORT const PlatformCapabilities platformCapabilities {
        false,
        3.0f
    };

    SystemVariable<ANativeWindow *> sNativeWindow = nullptr;
    AInputQueue *sQueue = nullptr;

    struct AndroidWindow final : OSWindow {
        AndroidWindow(ANativeWindow *window, WindowEventListener *listener)
            : OSWindow((uintptr_t)window, listener)
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

            bool handled = false;

            switch (action & AMOTION_EVENT_ACTION_MASK) {
            case AMOTION_EVENT_ACTION_DOWN:
                handled = injectPointerMove({ position, position,
                    position - mLastKnownMousePos });
                handled |= injectPointerPress({ position, position,
                    Input::MouseButton::LEFT_BUTTON });
                break;
            case AMOTION_EVENT_ACTION_UP:
                handled = injectPointerRelease({ position, position,
                    Input::MouseButton::LEFT_BUTTON });
                break;
            case AMOTION_EVENT_ACTION_MOVE:
                handled = injectPointerMove({ position, position,
                    position - mLastKnownMousePos });
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
                LOG_ERROR("Unknown Motion Event Type: " << (action & AMOTION_EVENT_ACTION_MASK));
                break;
            }

            mLastKnownMousePos = position;
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
            if (this)
                mHandle = 0;
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

        std::atomic_flag mResizeNeeded;
        bool mMinimized = false;

        InterfacesVector mContentPos;
        InterfacesVector mContentSize;
    };

    static std::optional<AndroidWindow> sWindow;

    template <auto f, typename... Args>
    static void delegate(ANativeActivity *activity, Args... args)
    {
        (*sWindow.*f)(args...);
    }

    void setup(ANativeActivity *activity)
    {
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

    void OSWindow::update()
    {
        if (sQueue) {
            AInputEvent *event = NULL;
            while (AInputQueue_getEvent(sQueue, &event) >= 0) {
                if (AInputQueue_preDispatchEvent(sQueue, event)) {
                    continue;
                }
                bool handled = false;
                switch (AInputEvent_getType(event)) {
                case AINPUT_EVENT_TYPE_KEY:
                    LOG_WARNING("Unhandled Key Event: " << AKeyEvent_getKeyCode(event));
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
        if (static_cast<AndroidWindow *>(this)->mResizeNeeded.test() && sWindow->mHandle != 0) {
            static_cast<AndroidWindow *>(this)->mResizeNeeded.clear();
            onResize(size());
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
        return static_cast<AndroidWindow*>(this)->mContentPos;
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
        sWindow.reset();
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

        MonitorInfo info { 0, 0, width, height };

        return { info };
    }
}
}

#endif
