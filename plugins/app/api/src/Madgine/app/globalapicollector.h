#pragma once

#include "Modules/uniquecomponent/uniquecomponentdefine.h"

DECLARE_UNIQUE_COMPONENT(Engine::Core, GlobalAPI, GlobalAPIBase, Engine::Plugins::Constructor<Application &>)

namespace Engine {
namespace Core {
    template <typename T>
    using GlobalAPI = Reflect::VirtualScope<T, GlobalAPIComponent<T>>;
}
}
