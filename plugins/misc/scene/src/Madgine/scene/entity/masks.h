#pragma once

namespace Engine
{
	namespace Scene
	{
		namespace Entity
		{
			namespace Masks
			{
				enum Masks
				{
					NO_MASK = 0x0,
					DEFAULT_MASK = 1u << 0,
					NAVMESH_MASK = 1u << 1,
					ENTITY_MASK = 1u << 2,
					TERRAIN_MASK = 1u << 3
				};
			}
		}
	}
}
