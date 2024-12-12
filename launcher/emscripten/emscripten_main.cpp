#include "Madgine/rootlib.h"

#include "../launcher.h"
#include "Interfaces/filesystem/fsapi.h"
#include "Madgine/root/root.h"
#include "Modules/threading/workgroup.h"
#include "Madgine/cli/cli.h"
#include <emscripten.h>

std::unique_ptr<Engine::CLI::CLICore> sTempCLI;

void mainImpl()
{
    emscripten_cancel_main_loop();
    static Engine::Threading::WorkGroup workGroup { "Launcher" };

    static Engine::Root::Root root { std::move(sTempCLI) };
    launch();
}

DLL_EXPORT_TAG int main(int argc, char **argv)
{
    sTempCLI = std::make_unique<Engine::CLI::CLICore>(argc, argv);

    void (*callback)() = &mainImpl;
    Engine::Filesystem::setup(&callback);

    emscripten_set_main_loop_arg([](void *) {}, nullptr, 0, false);
    return 0;
}
