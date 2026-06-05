#include "Madgine/applib.h"
#include "Madgine/resourceslib.h"

#include "main.h"

#include "Platform/filesystem/fsapi.h"

#include "Modules/threading/scheduler.h"
#include "Modules/threading/workgroup.h"

#include "Madgine/root/root.h"

#include "launcher.h"
#include "launcherconfig.h"
#include "main_compat.h"
#include "server.h"

int desktopMain_compat(int argc, char **argv)
{
    return desktopMain(argc, argv);
}

int desktopMain(int argc, char **argv, Engine::Closure<void(Engine::Core::Application &, Engine::Core::MainWindow &)> callback)
{
    Engine::Platform::Filesystem::setup();
    Engine::Threading::WorkGroup workGroup { "Launcher" };

    Engine::Core::Root root { argc, argv };

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