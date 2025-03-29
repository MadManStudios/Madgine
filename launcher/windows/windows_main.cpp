#include "Madgine/clientlib.h"

#include "../main.h"

#define NOMINMAX
#include <Windows.h>

#include <wincodec.h>

#include <thread>

#include "Madgine/window/mainwindow.h"

#include "launcherconfig.h"

static const char *CreateWindowClass(HINSTANCE hInstance)
{
    // Register class
    WNDCLASS wc {};
    wc.lpfnWndProc = DefWindowProc;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(hInstance, (LPCTSTR)IDI_APPLICATION);
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.lpszMenuName = nullptr;
    wc.lpszClassName = "LauncherSplashWindowClass";
    if (!RegisterClass(&wc))
        return nullptr;
    return wc.lpszClassName;
}

HWND CreateSplashWindow(HINSTANCE hInstance, const char *className)
{
    HWND hwndOwner = CreateWindow(className, NULL, WS_POPUP,
        0, 0, 0, 0, NULL, NULL, hInstance, NULL);
    return CreateWindowEx(WS_EX_LAYERED, className, NULL, WS_POPUP | WS_VISIBLE,
        0, 0, 0, 0, hwndOwner, NULL, hInstance, NULL);
}

void SetSplashImage(HWND hwndSplash, HBITMAP hbmpSplash)
{
    // get the size of the bitmap

    BITMAP bm;
    GetObject(hbmpSplash, sizeof(bm), &bm);
    SIZE sizeSplash = { bm.bmWidth, bm.bmHeight };

    // get the primary monitor's info

    POINT ptZero = { 0 };
    HMONITOR hmonPrimary = MonitorFromPoint(ptZero, MONITOR_DEFAULTTOPRIMARY);
    MONITORINFO monitorinfo = { 0 };
    monitorinfo.cbSize = sizeof(monitorinfo);
    GetMonitorInfo(hmonPrimary, &monitorinfo);

    // center the splash screen in the middle of the primary work area

    const RECT &rcWork = monitorinfo.rcWork;
    POINT ptOrigin;
    ptOrigin.x = rcWork.left + (rcWork.right - rcWork.left - sizeSplash.cx) / 2;
    ptOrigin.y = rcWork.top + (rcWork.bottom - rcWork.top - sizeSplash.cy) / 2;

    // create a memory DC holding the splash bitmap

    HDC hdcScreen = GetDC(NULL);
    HDC hdcMem = CreateCompatibleDC(hdcScreen);
    HBITMAP hbmpOld = (HBITMAP)SelectObject(hdcMem, hbmpSplash);

    // use the source image's alpha channel for blending

    BLENDFUNCTION blend = { 0 };
    blend.BlendOp = AC_SRC_OVER;
    blend.SourceConstantAlpha = 255;
    blend.AlphaFormat = AC_SRC_ALPHA;

    // paint the window (in the right location) with the alpha-blended bitmap

    UpdateLayeredWindow(hwndSplash, hdcScreen, &ptOrigin, &sizeSplash,
        hdcMem, &ptZero, RGB(255, 255, 255), &blend, ULW_OPAQUE);

    // delete temporary objects

    SelectObject(hdcMem, hbmpOld);
    DeleteDC(hdcMem);
    ReleaseDC(NULL, hdcScreen);
}

INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pCmdLine, int nCmdShow)
{
    Engine::Closure<void(Engine::App::Application &, Engine::Window::MainWindow &)> callback;

#ifdef MADGINE_LAUNCHER_SPLASH_IMAGE
    const char *className = CreateWindowClass(hInstance);

    HWND splashWindow = CreateSplashWindow(hInstance, className);

    HBITMAP bitmap = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(MADGINE_LAUNCHER_SPLASH_IMAGE), IMAGE_BITMAP, 0, 0, LR_DEFAULTCOLOR);

    SetSplashImage(splashWindow, bitmap);

    DeleteObject(bitmap);

    callback = [=](Engine::App::Application &app, Engine::Window::MainWindow &window) {
        window.taskQueue()->queue([=]() {
            DestroyWindow(GetParent(splashWindow));
        });
    };
#endif

    return desktopMain(__argc, __argv, std::move(callback));
}