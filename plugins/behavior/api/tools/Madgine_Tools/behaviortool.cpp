#include "behaviortoolslib.h"

#include "behaviortool.h"

#include "Modules/uniquecomponent/uniquecomponentcollector.h"

#include "Madgine/behavior/behavior.h"
#include "Madgine/behavior/behaviorcollector.h"
#include "Madgine/behavior/behaviorlist.h"

#include "Meta/keyvalue/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

#include "Madgine_Tools/debugger/debuggerview.h"
#include "Madgine_Tools/imguiicons.h"
#include "Madgine_Tools/inspector/inspector.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include "imgui/imguiaddons.h"

UNIQUECOMPONENT(Engine::Tools::BehaviorTool);

METATABLE_BEGIN_BASE(Engine::Tools::BehaviorTool, Engine::Tools::ToolBase)
METATABLE_END(Engine::Tools::BehaviorTool)

SERIALIZETABLE_INHERIT_BEGIN(Engine::Tools::BehaviorTool, Engine::Tools::ToolBase)
SERIALIZETABLE_END(Engine::Tools::BehaviorTool)

namespace Engine {
namespace Tools {

    TypedPtr visualizeCoroutineLocation(DebuggerView &view, const Debug::ContextInfo &context, const Behavior::CoroutineLocation &location, TypedPtr inlineLocation)
    {
        const char *name = "<unknown>";
#ifndef NDEBUG
        Debug::FullStackTrace trace = location.mStacktrace.calculateReadable();
        if (!trace.empty()) {
            name = trace[0].mFunction;
        }
#endif
        ImGui::BeginGroupPanel(name);
        TypedPtr content;
        if (location.mChild)
            content = view.visualizeDebugLocation(context, location.mChild, inlineLocation);
        ImGui::EndGroupPanel();

        return content == location.mChild ? &location : content;
    }

    BehaviorTool::BehaviorTool(ImRoot &root)
        : Tool<BehaviorTool>(root)
    {
    }

    Threading::Task<bool> BehaviorTool::init()
    {
        getTool<DebuggerView>().registerDebugLocationVisualizer<visualizeCoroutineLocation>();

        mInspector = &getTool<Inspector>();

        mInspector->addPreviewDefinition<Behavior::BehaviorList>([this](Behavior::BehaviorList *list) {
            return drawBehaviorList(*list);
        });

        co_return co_await ToolBase::init();
    }

    std::string_view BehaviorTool::key() const
    {
        return "BehaviorTool";
    }

    Threading::Task<void> BehaviorTool::finalize()
    {
        co_await ToolBase::finalize();
    }

    bool BehaviorTool::drawBehaviorList(Behavior::BehaviorList &list)
    {
        return std::erase_if(list.mEntries, [this](Behavior::BehaviorList::Entry &entry) {
            ImGui::BeginGroupPanel(entry.mHandle.name().data());
            ImGui::BeginTable("Entry", 2, ImGuiTableFlags_Resizable);
            mInspector->drawMembers(entry.mParameters.customScopePtr());
            ImGui::EndTable();

            ImGui::ItemSize({ ImGui::GetItemRectSize().x, 0 });

            ImGui::EndGroupPanel();

            bool remove = false;

            if (ImGui::BeginPopupCompoundContextItem()) {
                if (ImGui::MenuItem((IMGUI_ICON_X " Delete " + std::string { entry.mHandle.name() }).c_str())) {
                    remove = true;
                }
                ImGui::EndPopup();
            }
            return remove;
        }) > 0;
    }

    void BehaviorTool::renderMenu()
    {
    }

    Behavior::BehaviorHandle BehaviorSelector()
    {
        Behavior::BehaviorHandle result;

        for (auto [name, index] : Behavior::BehaviorFactoryRegistry::sComponentsByName()) {
            if (ImGui::BeginMenu(name.data())) {
                const Behavior::BehaviorFactoryBase *factory = Behavior::BehaviorFactoryRegistry::get(index).mFactory;
                for (std::string_view name : factory->names()) {
                    if (ImGui::MenuItem(name.data())) {
                        result = { index, std::string { name } };
                    }
                }
                ImGui::EndMenu();
            }
        }

        return result;
    }

}
}
