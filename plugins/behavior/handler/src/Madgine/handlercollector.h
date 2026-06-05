#pragma once

#include "Meta/reflect/virtualscope.h"

#include "Modules/uniquecomponent/uniquecomponentdefine.h"

DECLARE_UNIQUE_COMPONENT(Engine::Behavior, Handler, HandlerBase, Engine::Plugins::Constructor<HandlerManager &>)

namespace Engine {
namespace Behavior {

    template <typename T>
    using Handler = Reflect::VirtualScope<T, HandlerComponent<T>>;

}
}
