#include "../roottoolslib.h"

#include "keyvalueregistry.h"

#include "Madgine/root/keyvalueregistry.h"

#include "Meta/keyvalue/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

#include "Modules/uniquecomponent/uniquecomponentcollector.h"

#include "imgui/imgui.h"
#include "imgui/imguiaddons.h"

#include "Madgine_Tools/renderer/imroot.h"

#include "Madgine_Tools/inspector/inspector.h"

UNIQUECOMPONENT(Engine::Tools::KeyValueRegistry);

METATABLE_BEGIN_BASE(Engine::Tools::KeyValueRegistry, Engine::Tools::ToolBase)
METATABLE_END(Engine::Tools::KeyValueRegistry)

SERIALIZETABLE_INHERIT_BEGIN(Engine::Tools::KeyValueRegistry, Engine::Tools::ToolBase)
SERIALIZETABLE_END(Engine::Tools::KeyValueRegistry)

namespace Engine {
namespace Tools {

    KeyValueRegistry::KeyValueRegistry(ImRoot &root)
        : Tool<KeyValueRegistry>(root)
    {
    }

    Threading::Task<bool> KeyValueRegistry::init()
    {
        mInspector = &getTool<Inspector>();

        co_return co_await ToolBase::init();
    }

    void KeyValueRegistry::render()
    {
        if (ImGui::Begin("KeyValueRegistry", &mVisible)) {
            ImGui::SetWindowDockingDir(mRoot.dockSpaceId(), ImGuiDir_Left, 0.2f, false, ImGuiCond_FirstUseEver);

            auto drawList = [this](const std::map<std::string_view, ScopePtr> &items) {
                for (const std::pair<const std::string_view, ScopePtr> &p : items) {
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    if (ImGui::TreeNode(p.first.data())) {
                        mInspector->drawMembers(p.second, {});
                        ImGui::TreePop();
                    }
                }
            };

            if (ImGui::BeginTable("table", 2, ImGuiTableFlags_Resizable)) {
                drawList(Engine::KeyValueRegistry::globals());
                drawList(Engine::KeyValueRegistry::workgroupLocals());
                ImGui::EndTable();
            }
        }
        ImGui::End();
    }

    std::string_view KeyValueRegistry::key() const
    {
        return "KeyValueRegistry";
    }

}
}