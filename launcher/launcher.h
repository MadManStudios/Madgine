#pragma once

#include <functional>

#include "Generic/closure.h"

namespace Engine {
namespace Window {
    struct MainWindow;
}
namespace App {
    struct Application;
}
}

int launch(Engine::Closure<void(Engine::App::Application &, Engine::Window::MainWindow &)> callback = {});