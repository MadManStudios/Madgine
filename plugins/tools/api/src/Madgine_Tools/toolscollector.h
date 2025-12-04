#pragma once

#include "Meta/keyvalue/virtualscope.h"
#include "Meta/serialize/hierarchy/virtualserializableunit.h"

#include "Modules/uniquecomponent/uniquecomponentdefine.h"

DECLARE_UNIQUE_COMPONENT(Engine::Tools, Tools, ToolBase, Engine::UniqueComponent::Constructor<ImRoot &>)

namespace Engine {
namespace Tools {

    template <typename T, typename Base = ToolBase>
    using Tool = Serialize::VirtualUnit<T, VirtualScope<T, ToolsComponent<T, Base>>>;

    template <typename T>
    using ToolVirtualBase = ToolsVirtualBase<T>;

    template <typename T, typename Base>
    using ToolVirtualImpl = Serialize::VirtualUnit<T, VirtualScope<T, ToolsVirtualImpl<T, Base>>>;

}
}