#pragma once

#include "Modules/uniquecomponent/uniquecomponentdefine.h"

#include "Meta/keyvalue/virtualscope.h"

DECLARE_UNIQUE_COMPONENT(Engine, Handler, HandlerBase, Engine::UniqueComponent::Constructor<HandlerManager &>)

namespace Engine {

    template <typename T>
    using Handler = VirtualScope<T, HandlerComponent<T>>;

}
