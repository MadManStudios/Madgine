#pragma once

#include "Generic/closure.h"

namespace Engine{
namespace Window {
    struct MainWindow;
}
namespace App {
    struct Application;
}
}

int desktopMain(int argc, char **argv, Engine::Closure<void(Engine::App::Application &, Engine::Window::MainWindow &)> callback = {});