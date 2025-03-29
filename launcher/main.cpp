#include "Madgine/applib.h"
#include "Madgine/resourceslib.h"

#include "Interfaces/filesystem/fsapi.h"

#include "Modules/threading/workgroup.h"

#include "Madgine/root/root.h"
#include "Modules/threading/scheduler.h"

#include "launcherconfig.h"

#include "launcher.h"
#include "server.h"

#include "main.h"
#include "main_compat.h"

int desktopMain_compat(int argc, char **argv)
{
    return desktopMain(argc, argv);
}

int desktopMain(int argc, char **argv, Engine::Closure<void(Engine::App::Application &, Engine::Window::MainWindow &)> callback)
{
    Engine::Filesystem::setup();
    Engine::Threading::WorkGroup workGroup { "Launcher" };

    Engine::Root::Root root { argc, argv };

    if (root.errorCode() != 0)
        return root.errorCode();

    if (root.toolMode()) {
        int result = Engine::Threading::Scheduler {}.go();
        if (result != 0)
            return result;
        return root.errorCode();
    } else {
#if MADGINE_SERVER
        return server();
#else
        return launch(std::move(callback));
#endif
    }
}

#if !EMSCRIPTEN && !OSX && !IOS && !WINDOWS
DLL_EXPORT_TAG int main(int argc, char **argv)
{
    return desktopMain(argc, argv);
}
#endif