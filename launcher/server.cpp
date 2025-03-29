#include "Madgine/cli/clilib.h"
#include "Madgine/rootlib.h"
#include "Madgine/serverlib.h"

#include "server.h"


#include "Madgine/cli/parameter.h"

#include "Madgine/root/keyvalueregistry.h"

#include "Madgine/server/server.h"

#include "launcher.h"

#include "Modules/threading/scheduler.h"


#if EMSCRIPTEN
#    define FIX_LOCAL static
#else
#    define FIX_LOCAL
#endif

Engine::CLI::Parameter<bool> headlessParameter { { "--headless", "-l" }, false, "If set, the server will not have any graphical interface." };

int server_launch(Engine::Closure<void(Engine::App::Application &, Engine::Window::MainWindow &)> init)
{
    return launch(std::move(init));
}

int server() {
    FIX_LOCAL Engine::KeyValueWorkGroupLocal<Engine::Server::Server> server { "Server", server_launch };

    if (headlessParameter) {
        return Engine::Threading::Scheduler {}.go();
    } else {
        return launch();
    }
}