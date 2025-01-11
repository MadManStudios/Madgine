#pragma once

#include "Modules/uniquecomponent/uniquecomponentdefine.h"

#include "Meta/keyvalue/virtualscope.h"

DECLARE_UNIQUE_COMPONENT(Engine::Input, Handler, HandlerBase, Engine::UniqueComponent::Constructor<UIManager &>)

namespace Engine {
namespace Input {

    template <typename T>
    using Handler = VirtualScope<T, HandlerComponent<T>>;

    template <typename T>
    using WidgetHandler = VirtualScope<T, HandlerComponent<T, WidgetHandlerBase>>;

}
}
