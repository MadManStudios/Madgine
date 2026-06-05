#pragma once

#include "Generic/closure.h"

namespace Engine {
namespace Core {
    struct MainWindow;
    struct Application;
}
}

int launch(Engine::Closure<void(Engine::Core::Application &, Engine::Core::MainWindow &)> callback = {});