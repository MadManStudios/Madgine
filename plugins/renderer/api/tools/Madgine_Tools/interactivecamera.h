#pragma once

#include "imgui/imguiaddons.h"

namespace Engine {
namespace Tools {

    MADGINE_RENDER_TOOLS_EXPORT void InteractiveCamera(ImGui::InteractiveViewResultFlags flags, Render::Camera &camera);

}
}