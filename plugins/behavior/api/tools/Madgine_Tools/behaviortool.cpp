#include "behaviortoolslib.h"

#include "behaviortool.h"

#include "Modules/uniquecomponent/uniquecomponentcollector.h"

#include "Madgine/behavior/behavior.h"
#include "Madgine/behavior/behaviorcollector.h"
#include "Madgine/behavior/behaviorlist.h"

#include "Meta/reflect/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

#include "Madgine_Tools/debugger/debuggerview.h"
#include "Madgine_Tools/imguiicons.h"
#include "Madgine_Tools/inspector/inspector.h"
#include "Madgine_Tools/renderer/dialogs.h"

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

    std::vector<TypedPtr> visualizeCoroutineLocation(ContinuationList &continuations, DebuggerView &view, const Debug::ContextInfo &context, const Behavior::CoroutineLocation *location, TypedPtr inlineLocation)
    {
        if (!location)
            return {};

        const char *name = "<unknown>";
#ifndef NDEBUG
        Debug::FullStackTrace trace = location->mStacktrace.calculateReadable();
        if (!trace.empty()) {
            name = trace[0].mFunction;
        }
#endif
        ImGui::BeginGroupPanel(name);
        std::vector<TypedPtr> content;
        if (location->mChild) {
            if (BeginDebuggablePanel("Sender")) {
                content = view.visualizeDebugLocation(continuations, context, location->mChild, inlineLocation);
                EndDebuggablePanel();
            }
        }
        ImGui::EndGroupPanel();

        return content;
    }

    BehaviorTool::BehaviorTool(ImRoot &root)
        : Tool<BehaviorTool>(root)
    {
    }

    Threading::Task<bool> BehaviorTool::init()
    {
        getTool<DebuggerView>().registerDebugLocationVisualizer<visualizeCoroutineLocation>();

        mInspector = &getTool<Inspector>();

        mInspector->addPreviewDefinition<Behavior::BehaviorList>([this](const Traced<Behavior::BehaviorList *> &list) {
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

    bool BehaviorTool::drawBehaviorList(const Traced<Behavior::BehaviorList &> &list)
    {
        bool deleted = false;
        for (auto it = list.get().mEntries.begin(); it != list.get().mEntries.end();) {
            if ([&, this](Behavior::BehaviorList::Entry &entry) {
                    ImGui::BeginGroupPanel(entry.mHandle.name().data());
                    ImGui::BeginTable("Entry", 2, ImGuiTableFlags_Resizable);
                    mInspector->drawMembers(list.trace([](Behavior::BehaviorList &list, size_t index) { return list.mEntries[index].mParameters.customScopePtr(); }, std::distance(list.get().mEntries.begin(), it)));
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
                }(*it)) {
                it = list.get().mEntries.erase(it);
                deleted = true;
            } else {
                ++it;
            }
        }
        return deleted;
    }

    void BehaviorTool::renderMenu()
    {
    }

    Behavior::BehaviorHandle BehaviorSelector(ImGuiTextFilter **outFilter)
    {
        Behavior::BehaviorHandle result;

        static ImGuiTextFilter filter;
        if (outFilter)
            *outFilter = &filter;

        filter.Draw("##Filter", -FLT_MIN);
        if (ImGui::IsWindowAppearing())
            ImGui::SetKeyboardFocusHere(-1);

        for (auto [factoryName, index] : Behavior::BehaviorFactoryRegistry::sComponentsByName()) {

            bool hasMenu = false;
            const Behavior::BehaviorFactoryBase *factory = Behavior::BehaviorFactoryRegistry::get(index).mFactory;
            for (std::string_view name : factory->names()) {
                if (filter.PassFilter(name.data())) {

                    if (!hasMenu) {
                        hasMenu = ImGui::BeginMenu(factoryName.data());
                        if (!hasMenu)
                            break;
                    }

                    if (ImGui::MenuItem(name.data())) {
                        result = { index, std::string { name } };
                    }
                }
            }
            if (hasMenu)
                ImGui::EndMenu();
        }

        return result;
    }

    Dialog<Behavior::Behavior> BehaviorParameterDialog(Behavior::BehaviorHandle handle, Inspector &inspector)
    {
        UndoStack history;

        Behavior::ParameterTuple parameters = handle.createParameters();        

        DialogSettings &settings = co_await get_dialog_settings;

        settings.acceptText = "Create Behavior";

        do {
            history.handleShortcuts();

            if (ImGui::BeginTable("columns", 2, ImGuiTableFlags_SizingStretchProp)) {
                TracedRoot<Reflect::ScopePtr> traced { history, &parameters };
                inspector.drawMembers(traced);
                ImGui::EndTable();
            }
        } while (co_yield settings);

        co_return handle.create(parameters);
    }

}
}
