#include "../toolslib.h"

#include "imguidemo.h"

#include "Modules/uniquecomponent/uniquecomponentcollector.h"

#include "Meta/reflect/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

#include "imgui/imgui.h"

UNIQUECOMPONENT(Engine::Tools::ImGuiDemo);

METATABLE_BEGIN_BASE(Engine::Tools::ImGuiDemo, Engine::Tools::ToolBase)
METATABLE_END(Engine::Tools::ImGuiDemo)

SERIALIZETABLE_INHERIT_BEGIN(Engine::Tools::ImGuiDemo, Engine::Tools::ToolBase)
SERIALIZETABLE_END(Engine::Tools::ImGuiDemo)

namespace Engine {
namespace Tools {

    ImGuiDemo::ImGuiDemo(ImRoot &root)
        : Tool<ImGuiDemo>(root)
    {
    }

    void ImGuiDemo::renderMenu()
    {
        if (ImGui::BeginMenu("Dev")) {
            ImGui::MenuItem("ImGui Demo", nullptr, &mVisible);
            ImGui::EndMenu();
        }
    }

    void ImGuiDemo::render()
    {
        ImGui::ShowDemoWindow(&mVisible);
    }

    std::string_view ImGuiDemo::key() const
    {
        return "ImGuiDemo";
    }

}
}
