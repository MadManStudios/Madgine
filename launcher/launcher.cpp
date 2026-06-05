#include "Madgine/applib.h"
#include "Madgine/clientlib.h"

#include "launcher.h"

#include "Platform/filesystem/path.h"
#include "Platform/window/windowsettings.h"

#include "Modules/threading/scheduler.h"

#include "Madgine/app/application.h"
#include "Madgine/root/keyvalueregistry.h"
#include "Madgine/window/mainwindow.h"

#include "launcherconfig.h"

#if EMSCRIPTEN
#    define FIX_LOCAL static
#else
#    define FIX_LOCAL
#endif

#ifndef MADGINE_LAUNCHER_WINDOW_TITLE
#    define MADGINE_LAUNCHER_WINDOW_TITLE "Maditor"
#endif

int launch(Engine::Closure<void(Engine::Core::Application &, Engine::Core::MainWindow &)> callback)
{
    FIX_LOCAL Engine::Core::KeyValueWorkGroupLocal<Engine::Core::Application> app { "Application" };

    FIX_LOCAL Engine::Platform::Window::WindowSettings windowSettings;
    windowSettings.mTitle = MADGINE_LAUNCHER_WINDOW_TITLE;
    windowSettings.mIcon = MADGINE_LAUNCHER_ICON;
    FIX_LOCAL Engine::Core::KeyValueWorkGroupLocal<Engine::Core::MainWindow> window { "MainWindow", app, windowSettings };

    if (callback)
        callback(app, window);

    FIX_LOCAL Engine::Threading::Scheduler scheduler;
    int result = scheduler.go();
    LOG_DEBUG("Launcher finished with code " << result);
    return result;
}
