#pragma once

#include "Meta/reflect/virtualscope.h"
#include "Meta/serialize/hierarchy/virtualserializableunit.h"

#include "Modules/uniquecomponent/uniquecomponentdefine.h"

DECLARE_UNIQUE_COMPONENT(Engine::Tools, Tools, ToolBase, Engine::Plugins::Constructor<ImRoot &>)

namespace Engine {
namespace Tools {

    template <typename T, typename Base = ToolBase>
    using Tool = Serialize::VirtualUnit<T, Reflect::VirtualScope<T, ToolsComponent<T, Base>>>;

    template <typename T>
    using ToolVirtualBase = ToolsVirtualBase<T>;

    template <typename T, typename Base>
    using ToolVirtualImpl = Serialize::VirtualUnit<T, Reflect::VirtualScope<T, ToolsVirtualImpl<T, Base>>>;

}
}