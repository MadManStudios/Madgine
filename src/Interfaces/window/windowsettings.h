#pragma once

namespace Engine {
namespace Window {

    struct WindowData {
        InterfacesVector mPosition = { -1, -1 };
        InterfacesVector mSize = { 800, 600 };
        bool mMaximized = false;
    };

    struct WindowSettings {
        uintptr_t mHandle = 0;

        WindowData mData;

        const char *mTitle = "Interfaces - Window";

        bool mHidden = false;
        bool mHeadless = false;

        bool mRestoreGeometry = true;

        uintptr_t mIcon = 0;
    };

}
}