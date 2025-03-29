#pragma once

#include "Modules/uniquecomponent/uniquecomponentdefine.h"

#include "Meta/serialize/hierarchy/virtualserializableunit.h"

#include "Meta/keyvalue/virtualscope.h"

DECLARE_UNIQUE_COMPONENT(Engine::Tools, Tools, ToolBase, Engine::UniqueComponent::Constructor<ImRoot &>)

namespace Engine {
namespace Tools {

	template <typename T>
	using Tool = Serialize::VirtualUnit<T, VirtualScope<T, ToolsComponent<T>>>;

	template <typename T>
	using ToolVirtualBase = ToolsVirtualBase<T>;

	template <typename T, typename Base>
	using ToolVirtualImpl = Serialize::VirtualUnit<T, VirtualScope<T, ToolsVirtualImpl<T, Base>>>;

}
}