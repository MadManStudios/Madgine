#include "../toolslib.h"

#include "imguitestengine.h"

#include "Modules/uniquecomponent/uniquecomponentcollector.h"

#include "Meta/keyvalue/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

#include "imgui/imgui.h"
#include "imgui_test_engine/imgui_test_engine/imgui_te_ui.h"
#include "imroot.h"

UNIQUECOMPONENT(Engine::Tools::ImGuiTestEngine);

METATABLE_BEGIN_BASE(Engine::Tools::ImGuiTestEngine, Engine::Tools::ToolBase)
METATABLE_END(Engine::Tools::ImGuiTestEngine)

SERIALIZETABLE_INHERIT_BEGIN(Engine::Tools::ImGuiTestEngine, Engine::Tools::ToolBase)
SERIALIZETABLE_END(Engine::Tools::ImGuiTestEngine)

namespace Engine {
namespace Tools {

    ImGuiTestEngine::ImGuiTestEngine(ImRoot &root)
        : Tool<ImGuiTestEngine>(root)
    {
    }

    void ImGuiTestEngine::renderMenu()
    {
        if (ImGui::BeginMenu("Dev")) {
            ImGui::MenuItem("ImGui TestEngine", nullptr, &mVisible);
            ImGui::EndMenu();
        }
    }

    void ImGuiTestEngine::render()
    {
        ImGuiTestEngine_ShowTestEngineWindows(root().testEngine(), &mVisible);
    }

    std::string_view ImGuiTestEngine::key() const
    {
        return "ImGuiTestEngine";
    }

}
}
