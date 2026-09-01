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
#include "Madgine_Tools/texteditor/texteditor.h"

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

        ImGui::BeginGroupPanel(location->mLocation.function_name());
        std::vector<TypedPtr> content;

        if (ImGui::BeginTable("Code", 2, ImGuiTableFlags_SizingFixedFit)) {

            ImGui::TableSetupColumn("Line", 0);
            ImGui::TableSetupColumn("Source", ImGuiTableColumnFlags_WidthStretch);

            size_t baseLine = location->mLocation.line();
            IndexType<size_t> lineNr = location->mLine;

            ImGui::PushFont(view.getTool<TextEditor>().font());

            static std::map<std::string, std::vector<std::string>> sSourceCache;

            auto [it, b] = sSourceCache.try_emplace(location->mLocation.file_name());

            if (b) {
                std::ifstream ifs { location->mLocation.file_name() };
                std::string line;
                while (std::getline(ifs, line)) {
                    it->second.push_back(line);
                }
            }

            size_t bracketAcc = 0;
            bool wasGreaterZero = false;

            for (const std::string &line : it->second | std::ranges::views::drop(baseLine - 1)) {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text(std::to_string(baseLine));
                ImGui::TableNextColumn();

                float startY = ImGui::GetCursorScreenPos().y;

                ImGui::Text("%s", line.c_str());

                bool set = context.getBreakpoint(location, baseLine);
                if (Breakpoint(startY, ImGui::GetCursorScreenPos().y, &set))
                    context.setBreakpoint(location, baseLine, set);

                if (baseLine == lineNr) {
                    DrawDebugMarker(0.5f * (ImGui::GetCursorScreenPos().y + startY));

                    ImGui::PopFont();

                    if (location->mChild) {
                        if (BeginDebuggablePanel("Sender")) {
                            content = view.visualizeDebugLocation(continuations, context, location->mChild, inlineLocation);
                            EndDebuggablePanel();
                        }
                    }

                    ImGui::PushFont(view.getTool<TextEditor>().font());
                }

                baseLine++;

                bracketAcc += std::ranges::count(line, '{') - std::ranges::count(line, '}');
                wasGreaterZero |= bracketAcc > 0;
                if (bracketAcc + !wasGreaterZero == 0) {
                    break;
                }
            }

            ImGui::PopFont();

            ImGui::EndTable();
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
            if ([&, this](Behavior::BehaviorSender &sender) {
                    ImGui::BeginGroupPanel(sender.mHandle.name().data());
                    ImGui::BeginTable("Entry", 2, ImGuiTableFlags_Resizable);
                    mInspector->drawMembers(list.trace([](Behavior::BehaviorList &list, size_t index) { return list.mEntries[index].mParameters.customScopePtr(); }, std::distance(list.get().mEntries.begin(), it)));
                    ImGui::EndTable();

                    ImGui::ItemSize({ ImGui::GetItemRectSize().x, 0 });

                    ImGui::EndGroupPanel();

                    bool remove = false;

                    if (ImGui::BeginPopupCompoundContextItem()) {
                        if (ImGui::MenuItem((IMGUI_ICON_X " Delete " + std::string { sender.mHandle.name() }).c_str())) {
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

    Dialog<Behavior::BehaviorSender> BehaviorParameterDialog(Behavior::BehaviorHandle handle, Inspector &inspector)
    {
        UndoStack history;

        Behavior::BehaviorSender behavior = handle.sender();

        DialogSettings &settings = co_await get_dialog_settings;

        settings.acceptText = "Create Behavior";

        do {
            history.handleShortcuts();

            if (ImGui::BeginTable("columns", 2, ImGuiTableFlags_SizingStretchProp)) {
                TracedRoot<Reflect::ScopePtr> traced { history, &behavior.mParameters};
                inspector.drawMembers(traced);
                ImGui::EndTable();
            }
        } while (co_yield settings);

        co_return behavior;
    }

}
}
