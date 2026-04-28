#pragma once

struct ImGuiWindow;
struct ImGuiTestContext;

namespace Engine {
namespace Tools {

    MADGINE_TOOLS_EXPORT ImGuiWindow *SetToolWindowRef(ImGuiTestContext *ctx, std::string elementName, std::string windowName, std::string toolName);

}
}