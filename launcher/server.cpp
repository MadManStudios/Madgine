#include "Madgine/cli/clilib.h"
#include "Madgine/rootlib.h"
#include "Madgine/serverlib.h"

#include "server.h"

#include "Modules/threading/scheduler.h"

#include "Madgine/cli/parameter.h"
#include "Madgine/root/keyvalueregistry.h"
#include "Madgine/server/server.h"

#include "launcher.h"

#if EMSCRIPTEN
#    define FIX_LOCAL static
#else
#    define FIX_LOCAL
#endif

Engine::Core::Parameter<bool> headlessParameter { { "--headless", "-l" }, false, "If set, the server will not have any graphical interface." };

int server_launch(Engine::Closure<void(Engine::Core::Application &, Engine::Core::MainWindow &)> init)
{
    return launch(std::move(init));
}

int server()
{
    FIX_LOCAL Engine::Core::KeyValueWorkGroupLocal<Engine::Core::Server> server { "Server", server_launch };

    if (headlessParameter) {
        return Engine::Threading::Scheduler {}.go();
    } else {
        return launch();
    }
}