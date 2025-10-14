#pragma once

#include "../input/cursoricons.h"
#include "../input/inputevents.h"

#include "windowsettings.h"

namespace Engine {
namespace Window {

    struct PlatformCapabilities {
        bool mSupportMultipleWindows;
        float mScalingFactor;
    };

    struct ResizeEvent {
        InterfacesVector mSize;
    };

    struct CloseEvent {
    };

    struct RepaintEvent {
    };

    INTERFACES_EXPORT extern const PlatformCapabilities platformCapabilities;

    struct INTERFACES_EXPORT OSWindow {

        OSWindow(uintptr_t handle)
            : mHandle(handle)
        {
        }

        std::optional<WindowEvent> update() {
            if (mPendingEvents.empty()) {
                updateImpl();
            }
            if (!mPendingEvents.empty()) {
                WindowEvent event = mPendingEvents.front();
                mPendingEvents.erase(mPendingEvents.begin());
                return event;
            }
            return {};
        }

        uintptr_t intHandle() const
        {
            return mHandle;
        }

        void *ptrHandle() const
        {
            return reinterpret_cast<void *>(mHandle);
        }

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

        // Input
        bool isKeyDown(Input::Key::Key key);

        void captureInput();
        void releaseInput();
        void requestSoftwareKeyboard();
        void releaseSoftwareKeyboard();

        void setCursorIcon(Input::CursorIcon icon);

        // Clipboard
        static std::string getClipboardString();
        static bool setClipboardString(std::string_view s);

    protected:
        void updateImpl();

        void onEvent(const WindowEvent &event)
        {
            mPendingEvents.push_back(event);
        }

    protected:
        uintptr_t mHandle;

    private:        
        std::vector<WindowEvent> mPendingEvents;
    };

    INTERFACES_EXPORT OSWindow *sCreateWindow(const WindowSettings &settings);

    struct MonitorInfo {
        InterfacesVector mPosition;
        InterfacesVector mSize;
    };

    INTERFACES_EXPORT std::vector<MonitorInfo> listMonitors();

}
}
