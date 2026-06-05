#pragma once

#include "Generic/closure.h"

namespace Engine {
namespace Core {
    struct MainWindow;
    struct Application;
}
}

int desktopMain(int argc, char **argv, Engine::Closure<void(Engine::Core::Application &, Engine::Core::MainWindow &)> callback = {});