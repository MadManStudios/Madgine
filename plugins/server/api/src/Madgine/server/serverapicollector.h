#pragma once

#include "Modules/uniquecomponent/uniquecomponentdefine.h"

DECLARE_UNIQUE_COMPONENT(Engine::Core, ServerAPI, ServerAPIBase, Engine::Plugins::Constructor<Engine::Core::Server &>)

namespace Engine {
namespace Core {
    template <typename T>
    using ServerAPI = Reflect::VirtualScope<T, ServerAPIComponent<T>>;
}
}
