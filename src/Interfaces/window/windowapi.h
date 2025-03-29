#pragma once

#include "windoweventlistener.h"

#include "../input/cursoricons.h"
#include "../input/inputevents.h"

#include "windowsettings.h"

namespace Engine {
namespace Window {

    struct PlatformCapabilities {
        bool mSupportMultipleWindows;
        float mScalingFactor;
    };

    
    struct NoEvent { };

    struct ResizeEvent {
        InterfacesVector mSize;
    };

    struct CloseEvent {
    };

    struct RepaintEvent {
    };

    struct KeyPressEvent {
        Input::KeyEventArgs mEvent;
    };

    struct KeyReleaseEvent {
        Input::KeyEventArgs mEvent;
    };

    struct PointerPressEvent {
        Input::PointerEventArgs mEvent;
    };

    struct PointerReleaseEvent {
        Input::PointerEventArgs mEvent;
    };

    struct PointerMoveEvent {
        Input::PointerEventArgs mEvent;
    };

    struct AxisEvent {
        Input::AxisEventArgs mEvent;
    };

    using WindowEvent = std::variant<NoEvent, ResizeEvent, CloseEvent, RepaintEvent, KeyPressEvent, KeyReleaseEvent, PointerPressEvent, PointerReleaseEvent, PointerMoveEvent, AxisEvent>;


    INTERFACES_EXPORT extern const PlatformCapabilities platformCapabilities;

    struct INTERFACES_EXPORT OSWindow {

        OSWindow(uintptr_t handle, WindowEventListener *listener)
            : mHandle(handle)
            , mListener(listener)
        {
        }

        void update();

        InterfacesVector size();

        InterfacesVector renderSize();

        InterfacesVector pos();

        InterfacesVector renderPos();

        void setSize(const InterfacesVector &size);
        void setRenderSize(const InterfacesVector &size);

        void setPos(const InterfacesVector &pos);
        void setRenderPos(const InterfacesVector &pos);

        void show();
        bool isMinimized();
        bool isMaximized();

        void focus();
        bool hasFocus();

        void setTitle(const char *title);
        std::string title() const;

        void close();
        void destroy();

        WindowData data();

        //Input
        bool isKeyDown(Input::Key::Key key);

        void captureInput();
        void releaseInput();

        void setCursorIcon(Input::CursorIcon icon);

        //Clipboard
        static std::string getClipboardString();
        static bool setClipboardString(std::string_view s);

        uintptr_t mHandle;

    protected:
        void onResize(const InterfacesVector &size)
        {
            mListener->onResize(size);
        }

        void onClose()
        {
            mListener->onClose();
        }

        void onRepaint()
        {
            mListener->onRepaint();
        }

        //Input
        bool injectKeyPress(const Input::KeyEventArgs &arg)
        {
            return mListener->injectKeyPress(arg);
        }
        bool injectKeyRelease(const Input::KeyEventArgs &arg)
        {
            return mListener->injectKeyRelease(arg);
        }
        bool injectPointerPress(const Input::PointerEventArgs &arg)
        {
            captureInput();
            return mListener->injectPointerPress(arg);
        }
        bool injectPointerRelease(const Input::PointerEventArgs &arg)
        {
            releaseInput();
            return mListener->injectPointerRelease(arg);
        }
        bool injectPointerMove(const Input::PointerEventArgs &arg)
        {
            return mListener->injectPointerMove(arg);
        }
        bool injectAxisEvent(const Input::AxisEventArgs &arg)
        {
            return mListener->injectAxisEvent(arg);
        }

    private:
        WindowEventListener *mListener;
    };

    INTERFACES_EXPORT OSWindow *sCreateWindow(const WindowSettings &settings, WindowEventListener *listener);

    struct MonitorInfo {
        InterfacesVector mPosition;
        InterfacesVector mSize;
    };

    INTERFACES_EXPORT std::vector<MonitorInfo> listMonitors();

}
}
