#include "../toolslib.h"

#include "imguidemo.h"

#include "imgui/imgui.h"

#include "Meta/keyvalue/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

#include "Modules/uniquecomponent/uniquecomponentcollector.h"

namespace Engine {
namespace Tools {

    ImGuiDemo::ImGuiDemo(ImRoot &root)
        : Tool<ImGuiDemo>(root)
    {
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

UNIQUECOMPONENT(Engine::Tools::ImGuiDemo);

METATABLE_BEGIN_BASE(Engine::Tools::ImGuiDemo, Engine::Tools::ToolBase)
METATABLE_END(Engine::Tools::ImGuiDemo)

SERIALIZETABLE_INHERIT_BEGIN(Engine::Tools::ImGuiDemo, Engine::Tools::ToolBase)
SERIALIZETABLE_END(Engine::Tools::ImGuiDemo)

