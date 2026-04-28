#include "../toolslib.h"

#include "tooltests.h"

#include "imgui_test_engine/imgui_test_engine/imgui_te_context.h"

namespace Engine {
namespace Tools {

    ImGuiWindow *SetToolWindowRef(ImGuiTestContext *ctx, std::string elementName, std::string windowName, std::string toolName)
    {
        ctx->SetRef(ImGuiTestRef{ });
        auto window = ctx->GetWindowByRef((windowName + "##" + toolName).c_str());
        if (!window) {
            return nullptr;
        }

        ImGuiID dockspaceId = ImHashStr(toolName.c_str(), 0, window->DockId);
        std::string contentName = std::format("{}##{:x}", elementName, dockspaceId);
        ctx->SetRef(contentName.c_str());

        auto contentWindow = ImGui::FindWindowByName(contentName.c_str());
        return contentWindow;
    }

}
}