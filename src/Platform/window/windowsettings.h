#pragma once

namespace Engine {
namespace Platform {
    namespace Window {

        struct WindowData {
            PlatformVector mPosition = { -1, -1 };
            PlatformVector mSize = { 800, 600 };
            bool mMaximized = false;
        };

        struct WindowSettings {
            uintptr_t mHandle = 0;

            WindowData mData;

            const char *mTitle = "Platform - Window";

            bool mHidden = false;
            bool mHeadless = false;

            bool mRestoreGeometry = true;

            uintptr_t mIcon = 0;
        };

    }
}
}