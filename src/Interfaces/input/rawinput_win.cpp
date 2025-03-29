#include "../interfaceslib.h"

#if WINDOWS

#    include "rawinput_win.h"
#include "../helpers/win_ptrs.h"

#    if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)

namespace Engine {
namespace Input {

#    define USB_PACKET_LENGTH 64

    static std::mutex sRawInputDevicesMutex;
    static std::atomic_flag sRawInputInitialized = ATOMIC_FLAG_INIT;

    decltype(&HidD_GetManufacturerString) sGetManufacturerString;
    decltype(&HidD_GetProductString) sGetProductString;
    decltype(&HidD_GetPreparsedData) sGetPreparsedData;
    decltype(&HidP_GetCaps) sGetCaps;
    decltype(&HidP_GetValueCaps) sGetValueCaps;
    decltype(&HidP_GetButtonCaps) sGetButtonCaps;
    decltype(&HidP_GetUsageValue) sGetUsageValue;
    decltype(&HidP_GetData) sGetData;
    decltype(&HidP_MaxDataListLength) sMaxDataListLength;

    void loadHID(void)
    {
        HMODULE handle = LoadLibrary("hid.dll");
        assert(handle);

        sGetManufacturerString = (decltype(&HidD_GetManufacturerString))GetProcAddress(handle, "HidD_GetManufacturerString");
        sGetProductString = (decltype(&HidD_GetProductString))GetProcAddress(handle, "HidD_GetProductString");
        sGetPreparsedData = (decltype(&HidD_GetPreparsedData))GetProcAddress(handle, "HidD_GetPreparsedData");
        sGetCaps = (decltype(&HidP_GetCaps))GetProcAddress(handle, "HidP_GetCaps");
        sGetData = (decltype(&HidP_GetData))GetProcAddress(handle, "HidP_GetData");
        sGetValueCaps = (decltype(&HidP_GetValueCaps))GetProcAddress(handle, "HidP_GetValueCaps");
        sGetButtonCaps = (decltype(&HidP_GetButtonCaps))GetProcAddress(handle, "HidP_GetButtonCaps");
        sGetUsageValue = (decltype(&HidP_GetUsageValue))GetProcAddress(handle, "HidP_GetUsageValue");
        sMaxDataListLength = (decltype(&HidP_MaxDataListLength))GetProcAddress(handle, "HidP_MaxDataListLength");
    }

    std::map<HANDLE, RawInputDevice> sRawInputDevices;

    void addRawInputDevice(HANDLE device)
    {

        if (sRawInputDevices.count(device) > 0)
            return;

        RID_DEVICE_INFO rdi;
        UINT rdi_size = sizeof(rdi);

        auto result = GetRawInputDeviceInfoA(device, RIDI_DEVICEINFO, &rdi, &rdi_size);
        assert(result > 0);
        assert(rdi.dwType == RIM_TYPEHID);

        if (rdi.hid.usUsagePage != HID_USAGE_PAGE_GENERIC || (rdi.hid.usUsage != HID_USAGE_GENERIC_JOYSTICK && rdi.hid.usUsage != HID_USAGE_GENERIC_GAMEPAD))
            return;

        char device_name[MAX_PATH];
        UINT name_size = sizeof(device_name);
        result = GetRawInputDeviceInfoA(device, RIDI_DEVICENAME, device_name, &name_size);
        assert(result > 0);

        UniqueHandle hFile = CreateFileA(device_name, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
        assert(hFile != INVALID_HANDLE_VALUE);

        std::string manufacturer, product;
        {
            WCHAR string[128];

            if (sGetManufacturerString(hFile, string, sizeof(string))) {
                std::wstring ws = string;
                manufacturer = { ws.begin(), ws.end() };
            }
            if (sGetProductString(hFile, string, sizeof(string))) {
                std::wstring ws = string;
                product = { ws.begin(), ws.end() };
            }

            //device->name = SDL_CreateJoystickName(device->vendor_id, device->product_id, manufacturer_string, product_string);
        }

        PHIDP_PREPARSED_DATA preparsedData;
        result = sGetPreparsedData(hFile, &preparsedData);
        assert(result);

        //CHECK(SDL_HidD_GetPreparsedData(hFile, &device->preparsed_data));

        sRawInputDevices.try_emplace(device, device, std::move(manufacturer), std::move(product), preparsedData);
    }

    void listRawInputDevices()
    {
        std::scoped_lock lock { sRawInputDevicesMutex };

        UINT device_count = 0;

        if ((GetRawInputDeviceList(NULL, &device_count, sizeof(RAWINPUTDEVICELIST)) != -1) && device_count > 0) {
            std::unique_ptr<RAWINPUTDEVICELIST[]> devices = std::make_unique<RAWINPUTDEVICELIST[]>(device_count);

            if (GetRawInputDeviceList(devices.get(), &device_count, sizeof(RAWINPUTDEVICELIST)) != -1) {
                for (size_t i = 0; i < device_count; ++i) {
                    if (devices[i].dwType == RIM_TYPEHID)
                        addRawInputDevice(devices[i].hDevice);
                }
            }
        }
    }

    void setupRawInput(HWND handle)
    {
        if (!sRawInputInitialized.test_and_set()) {
            loadHID();
            listRawInputDevices();

            RAWINPUTDEVICE rid[2];

            rid[0].usUsagePage = HID_USAGE_PAGE_GENERIC;
            rid[0].usUsage = HID_USAGE_GENERIC_JOYSTICK;
            rid[0].dwFlags = RIDEV_DEVNOTIFY; /* Receive messages when in background, including device add/remove */
            rid[0].hwndTarget = NULL;

            rid[1].usUsagePage = HID_USAGE_PAGE_GENERIC;
            rid[1].usUsage = HID_USAGE_GENERIC_GAMEPAD;
            rid[1].dwFlags = RIDEV_DEVNOTIFY; /* Receive messages when in background, including device add/remove */
            rid[1].hwndTarget = NULL;

            auto result = RegisterRawInputDevices(rid, 2, sizeof(RAWINPUTDEVICE));
            assert(result);
        }
    }

    void handleRawInputStatePacket(RawInputDevice &device, BYTE *data, int bytesize, int count)
    {
        ULONG size = sMaxDataListLength(HidP_Input, device.mPreparsedData);
        std::unique_ptr<HIDP_DATA[]> list = std::make_unique<HIDP_DATA[]>(size);

        auto result = sGetData(HidP_Input, list.get(), &size, device.mPreparsedData, (PCHAR)data, bytesize);
        assert(result = HIDP_STATUS_SUCCESS);

        bool newButtonMask[16] = { 0 };

        for (size_t i = 0; i < size; ++i) {
            USHORT index = list[i].DataIndex;
            bool *flag = device.mCaps[index].mChangedFlag;
            switch (device.mCaps[index].mType) {
            case CapData::FLOAT:
                *flag |= *device.mCaps[index].mValue != list[i].RawValue / 32768.0f - 1.0f;
                *device.mCaps[index].mValue = list[i].RawValue / 32768.0f - 1.0f;
                break;
            case CapData::INT:
                *flag |= *device.mCaps[index].mIntValue != list[i].RawValue;
                *device.mCaps[index].mIntValue = list[i].RawValue;
                break;
            case CapData::BUTTON:
                newButtonMask[index - 1] = list[i].On;
                break;
            }
        }

        for (size_t i = 0; i < 16; ++i) {
            USHORT index = i + 1;
            if (device.mButtonMask[i] != newButtonMask[i]) {
                device.mKeyEvents.push({ KeyEventArgs { device.mCaps[index].mKey, 0 },
                    newButtonMask[i] ? RawInputDevice::DOWN : RawInputDevice::UP });
                device.mButtonMask[i] = newButtonMask[i];
            }
        }
    }

    RawInputDevice &handleRawInput(HRAWINPUT input)
    {
        std::byte data[sizeof(RAWINPUTHEADER) + sizeof(RAWHID) + USB_PACKET_LENGTH];
        UINT buffer_size = sizeof(data);

        auto result = GetRawInputData(input, RID_INPUT, data, &buffer_size, sizeof(RAWINPUTHEADER));
        assert(result > 0);
        PRAWINPUT raw_input = (PRAWINPUT)data;

        auto it = sRawInputDevices.find(raw_input->header.hDevice);
        if (it == sRawInputDevices.end()) {
            listRawInputDevices();
            it = sRawInputDevices.find(raw_input->header.hDevice);
        }
        assert(it != sRawInputDevices.end());
        handleRawInputStatePacket(it->second, raw_input->data.hid.bRawData, raw_input->data.hid.dwSizeHid, raw_input->data.hid.dwCount);
        return it->second;
    }

    RawInputDevice::RawInputDevice(HANDLE handle, std::string manufacturer, std::string product, PHIDP_PREPARSED_DATA preparsedData)
        : mHandle(handle)
        , mManufacturer(std::move(manufacturer))
        , mProduct(std::move(product))
        , mPreparsedData(preparsedData)
        , mControlSticks(2)
    {
        mControlSticks[0].mType = Input::AxisEventArgs::LEFT;
        mControlSticks[1].mType = Input::AxisEventArgs::RIGHT;

        HIDP_CAPS caps;
        auto result = sGetCaps(mPreparsedData, &caps);
        assert(result == HIDP_STATUS_SUCCESS);

        std::unique_ptr<HIDP_BUTTON_CAPS[]> buttonCaps = std::make_unique<HIDP_BUTTON_CAPS[]>(caps.NumberInputButtonCaps);

        result = sGetButtonCaps(HidP_Input, buttonCaps.get(), &caps.NumberInputButtonCaps, mPreparsedData);
        assert(result == HIDP_STATUS_SUCCESS);

        for (size_t i = 0; i < caps.NumberInputButtonCaps; ++i) {
            USHORT min, max;
            USAGE usage;
            if (buttonCaps[i].IsRange) {
                min = buttonCaps[i].Range.DataIndexMin;
                max = buttonCaps[i].Range.DataIndexMax;
                usage = buttonCaps[i].Range.UsageMin;
            } else {
                min = buttonCaps[i].NotRange.DataIndex;
                max = min;
                usage = buttonCaps[i].NotRange.Usage;
            }
            if (mCaps.size() < max + 1)
                mCaps.resize(max + 1);
            for (USHORT index = min; index <= max; ++index) {
                mCaps[index] = resolveButtonCap(usage++);
            }
        }

        std::unique_ptr<HIDP_VALUE_CAPS[]> valueCaps = std::make_unique<HIDP_VALUE_CAPS[]>(caps.NumberInputValueCaps);

        result = sGetValueCaps(HidP_Input, valueCaps.get(), &caps.NumberInputValueCaps, mPreparsedData);
        assert(result == HIDP_STATUS_SUCCESS);

        for (size_t i = 0; i < caps.NumberInputValueCaps; ++i) {
            assert(!valueCaps[i].IsRange);
            USHORT index = valueCaps[i].NotRange.DataIndex;
            if (mCaps.size() < index + 1)
                mCaps.resize(index + 1);
            mCaps[index] = resolveValueCap(valueCaps[i].NotRange.Usage);
        }
    }

    CapData RawInputDevice::resolveValueCap(USAGE usage)
    {

        switch (usage) {
        case HID_USAGE_GENERIC_X:
            return { CapData::FLOAT, &mControlSticks[0].mChanged, { &mControlSticks[0].mAxis1 } };
        case HID_USAGE_GENERIC_Y:
            return { CapData::FLOAT, &mControlSticks[0].mChanged, { &mControlSticks[0].mAxis2 } };
        case HID_USAGE_GENERIC_RX:
            return { CapData::FLOAT, &mControlSticks[1].mChanged, { &mControlSticks[1].mAxis1 } };
        case HID_USAGE_GENERIC_RY:
            return { CapData::FLOAT, &mControlSticks[1].mChanged, { &mControlSticks[1].mAxis2 } };
        case HID_USAGE_GENERIC_Z:
            return { CapData::FLOAT, &mZAxis.mChanged, { &mZAxis.mValue } };
        case HID_USAGE_GENERIC_HATSWITCH: {
            CapData result { CapData::INT, &mDPad.mChanged };
            result.mIntValue = &mDPad.mValue;
            return result;
        }
        default:
            LOG("unknown value usage: " << usage);
            return { CapData::FLOAT, & mUnknownValueDummy.dummy1, { &mUnknownValueDummy.dummy2 } };
        }
    }

    CapData RawInputDevice::resolveButtonCap(USAGE usage)
    {
        CapData data = { CapData::BUTTON, nullptr };

        if (usage > 16) {
            LOG("unknown button usage: " << usage);
            data.mKey = static_cast<Key::Key>(0);
        } else if (usage > 8)
            data.mKey = static_cast<Key::Key>(0x98 + usage - 9);
        else
            data.mKey = static_cast<Key::Key>(0x88 + usage - 1);
        return data;
    }

    bool RawInputDevice::fetchAxisEvent(AxisEventArgs &arg)
    {
        for (ControlStick &stick : mControlSticks) {
            if (stick.mChanged) {
                arg.mAxisType = stick.mType;
                arg.mAxis1 = stick.mAxis1;
                arg.mAxis2 = stick.mAxis2;
                stick.mChanged = false;
                return true;
            }
        }

        if (mZAxis.mChanged) {
            arg.mAxisType = AxisEventArgs::Z;
            arg.mAxis1 = mZAxis.mValue;
            mZAxis.mChanged = false;
            return true;
        }

        if (mDPad.mChanged) {
            arg.mAxisType = AxisEventArgs::DPAD;
            arg.mAxis1 = mDPad.mValue;
            mDPad.mChanged = false;
            return true;
        }

        return false;
    }

    bool RawInputDevice::fetchKeyEvent(KeyEventArgs &arg, Dir &dir)
    {
        if (!mKeyEvents.empty()) {
            arg = mKeyEvents.front().first;
            dir = mKeyEvents.front().second;
            mKeyEvents.pop();
            return true;
        } else {
            return false;
        }
    }

}
}

#endif

#endif
