#pragma once

#include "Madgine/render/vertex.h"

namespace Engine {
namespace Widgets {

	using Vertex = Compound<
		Render::VertexPos, 
		Render::VertexColor, 
		Render::VertexUV>;

}
}