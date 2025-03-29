#include "../interfaceslib.h"

#if WINDOWS

#    include "windowapi.h"
#    include "windoweventlistener.h"
#    include "windowsettings.h"

#    include "../input/inputevents.h"
#    include "../input/rawinput_win.h"

#    define NOMINMAX
#    include <Windows.h>
#    include <windowsx.h>

#    if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)

namespace Engine {
namespace Window {

    DLL_EXPORT const PlatformCapabilities platformCapabilities {
        true,
        1.0f
    };

    struct WindowsWindow final : OSWindow {
        WindowsWindow(HWND hwnd, WindowEventListener *listener)
            : OSWindow((uintptr_t)hwnd, listener)
            , mKeyDown {}
        {
        }

        bool handle(UINT msg, WPARAM wParam, LPARAM lParam, bool &ignore)
        {
            if (msg >= WM_MOUSEFIRST && msg <= WM_MOUSELAST) {
                InterfacesVector windowPos = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
                InterfacesVector screenPos = windowPos + renderPos();
                switch (msg) {
                case WM_LBUTTONDOWN:
                    injectPointerPress(Input::PointerEventArgs { windowPos, screenPos, Input::MouseButton::LEFT_BUTTON });
                    break;
                case WM_LBUTTONUP:
                    injectPointerRelease(Input::PointerEventArgs { windowPos, screenPos, Input::MouseButton::LEFT_BUTTON });
                    break;
                case WM_RBUTTONDOWN:
                    injectPointerPress(Input::PointerEventArgs { windowPos, screenPos, Input::MouseButton::RIGHT_BUTTON });
                    break;
                case WM_RBUTTONUP:
                    injectPointerRelease(Input::PointerEventArgs { windowPos, screenPos, Input::MouseButton::RIGHT_BUTTON });
                    break;
                case WM_MBUTTONDOWN:
                    injectPointerPress(Input::PointerEventArgs { windowPos, screenPos, Input::MouseButton::MIDDLE_BUTTON });
                    break;
                case WM_MBUTTONUP:
                    injectPointerRelease(Input::PointerEventArgs { windowPos, screenPos, Input::MouseButton::MIDDLE_BUTTON });
                    break;
                case WM_MOUSEMOVE:
                    injectPointerMove(Input::PointerEventArgs { windowPos, screenPos, windowPos - mLastKnownMousePos });
                    mLastKnownMousePos = windowPos;
                    break;
                case WM_MOUSEWHEEL:
                    injectAxisEvent(Input::AxisEventArgs { Input::AxisEventArgs::WHEEL, GET_WHEEL_DELTA_WPARAM(wParam) / float(WHEEL_DELTA) });
                    break;
                }
            } else if (msg >= WM_KEYFIRST && msg <= WM_KEYLAST) {
                Input::Key::Key keycode = (Input::Key::Key)wParam;
                UINT scancode = (lParam >> 16) & 0xFF;
                switch (msg) {
                case WM_KEYDOWN:
                case WM_SYSKEYDOWN:
                    mKeyDown[keycode] = std::numeric_limits<BYTE>::max();
                    WORD buffer;
                    switch (keycode) {
                    case Input::Key::Key::Return:
                        buffer = '\n';
                        break;
                    default:
                        ToAscii(keycode, scancode, mKeyDown, &buffer, 0);
                    }
                    injectKeyPress(Input::KeyEventArgs { keycode, static_cast<char>(buffer), controlKeyState() });
                    break;
                case WM_KEYUP:
                case WM_SYSKEYUP:
                    mKeyDown[keycode] = 0;
                    injectKeyRelease(Input::KeyEventArgs { keycode, 0, controlKeyState() });
                    break;
                default:
                    LOG("Unknown KeyEvent " << msg);
                }
            } else {
                switch (msg) {
                case WM_SIZE:
                    mMinimized = wParam == SIZE_MINIMIZED;
                    if (lParam > 0)
                        onResize({ LOWORD(lParam), HIWORD(lParam) });
                    break;
                case WM_CLOSE:
                    onClose();
                    break;
                case WM_DESTROY:
                    return false;
                    break;
                case WM_PAINT: {
                    PAINTSTRUCT ps;
                    BeginPaint((HWND)mHandle, &ps);
                    onRepaint();
                    EndPaint((HWND)mHandle, &ps);
                    break;
                }
                case WM_INPUT: {
                    Input::RawInputDevice &device = Input::handleRawInput((HRAWINPUT)lParam);
                    Input::AxisEventArgs axis;
                    while (device.fetchAxisEvent(axis))
                        injectAxisEvent(axis);
                    Input::KeyEventArgs key;
                    Input::RawInputDevice::Dir dir;
                    while (device.fetchKeyEvent(key, dir)) {
                        switch (dir) {
                        case Input::RawInputDevice::UP:
                            injectKeyRelease(key);
                            break;
                        case Input::RawInputDevice::DOWN:
                            injectKeyPress(key);
                            break;
                        }
                    }
                    break;
                }
                case WM_SYSCOMMAND:
                    if (wParam == SC_KEYMENU)
                        ignore = true;
                    break;
                case WM_SETCURSOR:
                    break;
                    //default:
                    //LOG_WARNING("Unhandled Event type: " << msg);
                }
            }

            return true;
        }

        Input::ControlKeyState controlKeyState() const
        {
            return {
                (bool)mKeyDown[Input::Key::Shift],
                (bool)mKeyDown[Input::Key::Control],
                (bool)mKeyDown[Input::Key::Alt]
            };
        }

        BYTE mKeyDown[512];
        InterfacesVector mLastKnownMousePos;
        bool mMinimized = false;
    };

    void OSWindow::update()
    {
        MSG msg;
        while (PeekMessage(&msg, (HWND)mHandle, 0U, 0U, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    InterfacesVector OSWindow::size()
    {
        RECT rect;
        auto result = GetWindowRect((HWND)mHandle, &rect);
        assert(result);
        return { rect.right - rect.left, rect.bottom - rect.top };
    }

    InterfacesVector OSWindow::renderSize()
    {
        RECT rect;
        auto result = GetClientRect((HWND)mHandle, &rect);
        assert(result);
        return { rect.right - rect.left, rect.bottom - rect.top };
    }

    void OSWindow::close()
    {
        PostMessage((HWND)mHandle, WM_CLOSE, 0, 0);
    }

    void OSWindow::destroy()
    {
        DestroyWindow((HWND)mHandle);
    }

    InterfacesVector OSWindow::pos()
    {
        RECT rect;
        auto result = GetWindowRect((HWND)mHandle, &rect);
        assert(result);
        return { rect.left, rect.top };
    }

    InterfacesVector OSWindow::renderPos()
    {
        POINT p { 0, 0 };
        auto result = ClientToScreen((HWND)mHandle, &p);
        assert(result);
        return { p.x, p.y };
    }

    void OSWindow::setSize(const InterfacesVector &size)
    {
        MoveWindow((HWND)mHandle, pos().x, pos().y, size.x, size.y, true);
    }

    void OSWindow::setRenderSize(const InterfacesVector &size)
    {
        RECT r;
        auto result = GetClientRect((HWND)mHandle, &r);
        assert(result);
        RECT r2;
        result = GetWindowRect((HWND)mHandle, &r2);
        assert(result);
        MoveWindow((HWND)mHandle, pos().x, pos().y, size.x + ((r2.right - r2.left) - (r.right - r.left)), size.y + ((r2.bottom - r2.top) - (r.bottom - r.top)), true);
    }

    void OSWindow::setPos(const InterfacesVector &pos)
    {
        MoveWindow((HWND)mHandle, pos.x, pos.y, size().x, size().y, true);
    }

    void OSWindow::setRenderPos(const InterfacesVector &pos)
    {
        RECT r;
        auto result = GetWindowRect((HWND)mHandle, &r);
        assert(result);
        POINT p { 0, 0 };
        result = ClientToScreen((HWND)mHandle, &p);
        assert(result);
        result = MoveWindow((HWND)mHandle, pos.x - (p.x - r.left), pos.y - (p.y - r.top), size().x, size().y, true);
        assert(result);
    }

    void OSWindow::show()
    {
        ShowWindow((HWND)mHandle, SW_SHOW);
    }

    bool OSWindow::isMinimized()
    {
        return static_cast<WindowsWindow *>(this)->mMinimized;
    }

    bool OSWindow::isMaximized()
    {
        WINDOWPLACEMENT placement;
        auto result = GetWindowPlacement((HWND)mHandle, &placement);
        assert(result);
        return placement.showCmd == SW_MAXIMIZE;
    }

    void OSWindow::focus()
    {
        SetFocus((HWND)mHandle);
    }

    bool OSWindow::hasFocus()
    {
        return GetFocus() == (HWND)mHandle;
    }

    void OSWindow::setTitle(const char *title)
    {
        SetWindowTextA((HWND)mHandle, title);
    }

    std::string OSWindow::title() const
    {
        std::string result;
        result.resize(256);
        result.resize(GetWindowTextA((HWND)mHandle, result.data(), 256));
        return result;
    }

    //Input
    bool OSWindow::isKeyDown(Input::Key::Key key)
    {
        return static_cast<WindowsWindow *>(this)->mKeyDown[key];
    }

    void OSWindow::captureInput()
    {
        SetCapture((HWND)mHandle);
    }

    void OSWindow::releaseInput()
    {
        ReleaseCapture();
    }

    void OSWindow::setCursorIcon(Input::CursorIcon icon)
    {
        SetCursor(LoadCursor(NULL, [](Input::CursorIcon icon) {
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
        }(icon)));
    }

    std::string OSWindow::getClipboardString()
    {
        std::string result;
        if (OpenClipboard(NULL)) {
            HANDLE handle = GetClipboardData(CF_TEXT);
            if (handle != NULL) {
                result = (const char *)GlobalLock(handle);
                GlobalUnlock(handle);
            }
            CloseClipboard();
        }
        return result;
    }

    bool OSWindow::setClipboardString(std::string_view s)
    {
        if (!OpenClipboard(NULL))
            return false;

        HGLOBAL handle = GlobalAlloc(GMEM_MOVEABLE, s.size() + 1);
        if (handle == NULL) {
            CloseClipboard();
            return false;
        }
        char *buffer = (char *)GlobalLock(handle);
        std::strncpy(buffer, s.data(), s.size());
        GlobalUnlock(handle);
        EmptyClipboard();
        if (SetClipboardData(CF_TEXT, handle) == NULL) {
            GlobalFree(handle);
            CloseClipboard();
            return false;
        }
        CloseClipboard();
        return true;
    }

    WindowData OSWindow::data()
    {
        WINDOWPLACEMENT wndpl = { sizeof(WINDOWPLACEMENT) };
        GetWindowPlacement((HWND)mHandle, &wndpl);

        return {
            { wndpl.rcNormalPosition.left, wndpl.rcNormalPosition.top },
            { wndpl.rcNormalPosition.right - wndpl.rcNormalPosition.left,
                wndpl.rcNormalPosition.bottom - wndpl.rcNormalPosition.top },
            wndpl.showCmd == SW_MAXIMIZE
        };
    }

    static std::unordered_map<HWND, WindowsWindow> sWindows;

    LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    static const char *CreateWindowClass()
    {
        HINSTANCE hInstance = GetModuleHandle(nullptr);

        // Register class
        WNDCLASSEX wcex;
        wcex.cbSize = sizeof(WNDCLASSEX);
        wcex.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
        wcex.lpfnWndProc = WndProc;
        wcex.cbClsExtra = 0;
        wcex.cbWndExtra = 0;
        wcex.hInstance = hInstance;
        wcex.hIcon = LoadIcon(hInstance, (LPCTSTR)IDI_APPLICATION);
        wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        wcex.lpszMenuName = nullptr;
        wcex.lpszClassName = "InterfacesWindowClass";
        wcex.hIconSm = LoadIcon(wcex.hInstance, (LPCTSTR)IDI_APPLICATION);
        if (!RegisterClassEx(&wcex))
            return nullptr;
        return wcex.lpszClassName;
    }

    OSWindow *sCreateWindow(const WindowSettings &settings, WindowEventListener *listener)
    {
        HWND handle = (HWND)settings.mHandle;
        if (!handle) {
            static const char *windowClass = CreateWindowClass();

            DWORD style = WS_OVERLAPPEDWINDOW;
            if (settings.mHeadless) {
                style = WS_POPUP;
            }

            // Create window
            HINSTANCE hInstance = GetModuleHandle(nullptr);
            RECT rc = { 0, 0, (LONG)settings.mData.mSize.x, (LONG)settings.mData.mSize.y };
            AdjustWindowRect(&rc, style, FALSE);
            int x = CW_USEDEFAULT, y = CW_USEDEFAULT;
            if (settings.mData.mPosition.x >= 0 && settings.mData.mPosition.y >= 0) {
                x = settings.mData.mPosition.x;
                y = settings.mData.mPosition.y;
            }

            handle = CreateWindow(windowClass, settings.mTitle,
                style,
                x, y, rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, hInstance,
                nullptr);
            if (!handle)
                return nullptr;
        }

        if (settings.mIcon) {
            HICON icon = (HICON)LoadImage(GetModuleHandle(nullptr), (LPCSTR)settings.mIcon, IMAGE_ICON, 0, 0, LR_DEFAULTCOLOR);
            SendMessage(handle, WM_SETICON, ICON_SMALL, (LPARAM)icon);
            SendMessage(handle, WM_SETICON, ICON_BIG, (LPARAM)icon);
            //DestroyIcon(icon);
        }

        if (!settings.mHidden) {
            UINT showCmd = settings.mData.mMaximized ? SW_SHOWMAXIMIZED : SW_SHOW;
            if (settings.mData.mPosition.x >= 0 && settings.mData.mPosition.y >= 0) {
                WINDOWPLACEMENT wndpl {
                    sizeof(WINDOWPLACEMENT),
                    0,
                    showCmd,
                    { -1, -1 },
                    { -1, -1 },
                    { settings.mData.mPosition.x, settings.mData.mPosition.y,
                        settings.mData.mPosition.x + settings.mData.mSize.x,
                        settings.mData.mPosition.y + settings.mData.mSize.y }
                };
                SetWindowPlacement(handle, &wndpl);
            } else {
                ShowWindow(handle, showCmd);
            }
        }

        Input::setupRawInput(handle);

        auto pib = sWindows.try_emplace(handle, handle, listener);
        assert(pib.second);

        return &pib.first->second;
    }

    static std::vector<MonitorInfo> sBuffer;

    static BOOL __stdcall MonitorEnumerator(HMONITOR monitor, HDC, LPRECT, LPARAM)
    {
        MONITORINFO info;
        info.cbSize = sizeof(info);
        auto result = GetMonitorInfo(monitor, &info);
        assert(result);

        LPRECT rect = &info.rcWork;
        sBuffer.push_back({ rect->left, rect->top, rect->right - rect->left, rect->bottom - rect->top });
        return BOOL(true);
    }

    static void updateMonitors()
    {
        sBuffer.clear();
        auto result = EnumDisplayMonitors(
            NULL, NULL, &MonitorEnumerator,
            NULL);
        assert(result);
    }

    LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        bool ignore = false;
        auto it = sWindows.find(hwnd);
        if (it != sWindows.end()) {
            if (msg == WM_DISPLAYCHANGE) {
                if (it == sWindows.begin())
                    updateMonitors();
            } else if (!it->second.handle(msg, wParam, lParam, ignore))
                sWindows.erase(it);
        }
        if (ignore)
            return 0;
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }

    std::vector<MonitorInfo> listMonitors()
    {
        if (sBuffer.empty())
            updateMonitors();
        return sBuffer;
    }
}
}

#    endif

#endif
