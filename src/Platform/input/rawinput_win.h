#pragma once

#if WINDOWS

#    define NOMINMAX
#    include <Windows.h>
#    include <hidsdi.h>
#    include <queue>

#    include "inputevents.h"

#    if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)

namespace Engine {
namespace Platform {
    namespace Input {

        struct CapData {
            enum {
                BUTTON,
                FLOAT,
                INT
            } mType;
            bool *mChangedFlag;
            union {
                float *mValue;
                int *mIntValue;
                Input::Key::Key mKey;
            };
        };

        struct RawInputDevice {
            RawInputDevice(HANDLE handle, std::string manufacturer, std::string product, PHIDP_PREPARSED_DATA preparsedData);

            CapData resolveValueCap(USAGE usage);

            CapData resolveButtonCap(USAGE usage);

            std::variant<NoEvent, AxisEvent, KeyPressEvent, KeyReleaseEvent> fetchEvent();

            HANDLE mHandle;
            std::string mManufacturer;
            std::string mProduct;

            PHIDP_PREPARSED_DATA mPreparsedData;

            std::vector<CapData> mCaps;

            struct ControlStick {
                AxisEvent::AxisType mType;
                bool mChanged;
                float mAxis1, mAxis2;
            };

            std::vector<ControlStick> mControlSticks;

            std::queue<std::variant<KeyPressEvent, KeyReleaseEvent>> mKeyEvents;
            bool mButtonMask[16] = { 0 };

            struct ZAxis {
                bool mChanged;
                float mValue;
            } mZAxis;

            struct DPad {
                bool mChanged;
                int mValue;
            } mDPad;

            struct UnknownValueInput {
                bool dummy1;
                float dummy2;
            } mUnknownValueDummy;
        };

        void setupRawInput(HWND handle);
        Input::RawInputDevice &handleRawInput(HRAWINPUT input);

    }
}
}

#    endif

#endif