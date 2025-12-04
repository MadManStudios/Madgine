#pragma once

#include "Meta/keyvalue/virtualscope.h"

#include "Modules/uniquecomponent/uniquecomponentdefine.h"

DECLARE_UNIQUE_COMPONENT(Engine::Behavior, Handler, HandlerBase, Engine::UniqueComponent::Constructor<HandlerManager &>)

namespace Engine {
namespace Behavior {

    template <typename T>
    using Handler = VirtualScope<T, HandlerComponent<T>>;

}
}
