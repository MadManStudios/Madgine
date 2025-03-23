#pragma once

#include "Modules/uniquecomponent/uniquecomponentdefine.h"

DECLARE_UNIQUE_COMPONENT(Engine::Server, ServerAPI, ServerAPIBase, Engine::UniqueComponent::Constructor<Engine::Server::Server&>)

namespace Engine
{
	namespace Server
	{
    template <typename T>
		using ServerAPI = VirtualScope<T, ServerAPIComponent<T>>;
	}
}
