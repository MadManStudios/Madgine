#include "../templateslib.h"

#include "templates.h"

#include "Modules/uniquecomponent/uniquecomponentcollector.h"

#include "Meta/serialize/serializetable_impl.h"

#include "Meta/keyvalue/metatable_impl.h"

#include "imgui/imgui.h"
#include "imgui/imguiaddons.h"

#include "Interfaces/filesystem/fsapi.h"

#include "Madgine_Tools/inspector/inspector.h"
#include "Madgine_Tools/renderer/imroot.h"

#include "TemplateEngine/parser.h"

UNIQUECOMPONENT(Engine::Tools::Templates)

METATABLE_BEGIN_BASE(Engine::Tools::Templates, Engine::Tools::ToolBase)
METATABLE_END(Engine::Tools::Templates)

SERIALIZETABLE_INHERIT_BEGIN(Engine::Tools::Templates, Engine::Tools::ToolBase)
SERIALIZETABLE_END(Engine::Tools::Templates)

namespace Engine {
namespace Tools {

    Templates::Templates(ImRoot &root)
        : Tool<Templates>(root)
    {
    }

    Threading::Task<bool> Templates::init()
    {
        if (!co_await ToolBase::init())
            co_return false;

        mInspector = &getTool<Inspector>();

        co_return true;
    }

    void Templates::renderMenu()
    {
        ToolBase::renderMenu();

        if (mVisible) {

            if (ImGui::BeginMenu("Templates")) {

                for (Filesystem::FileQueryResult dir : Filesystem::listDirs(PROJECT_ROOT "/templates")) {
                    std::string name = dir.path().filename().str();
                    if (ImGui::MenuItem(name.c_str())) {
                        showTemplateDialog(name);
                    }
                }

                ImGui::EndMenu();
            }
        }
    }

    std::string_view Templates::key() const
    {
        return "Templates";
    }

    void Templates::showTemplateDialog(std::string_view name, Closure<void(const Filesystem::Path &)> cb)
    {
        Filesystem::Path dir = Filesystem::Path { PROJECT_ROOT "/templates" } / name;

        mRoot.dialogs().show(
            [](Templates *templates, Filesystem::Path path) -> Dialog<TemplateEngine::Parser, Filesystem::Path> {
                Filesystem::Path outputPath { "C:\\Users\\Bub\\Desktop\\Test" };

                TemplateEngine::Parser parser { path };

                DialogSettings &settings = co_await get_dialog_settings;
                settings.header = "Generate 'Test'";
                do {
                    if (ImGui::BeginTable("fields", 2, ImGuiTableFlags_Resizable)) {
                        for (auto &[key, value] : parser.fields()) {
                            templates->mInspector->drawValue(key, value, true, false);
                        }
                        ImGui::EndTable();
                    }

                    ImGui::Text("Target:");
                    std::string s = outputPath;
                    if (ImGui::InputText("###Target", &s)) {
                        outputPath = s;
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("...")) {
                        std::optional<std::tuple<Filesystem::Path>> result = co_await templates->mRoot.directoryPicker();
                        if (result) {
                            outputPath = std::get<0>(*result);
                        }
                    }

                } while (co_yield settings);
                co_return { std::move(parser), std::move(outputPath) };
            }(this, dir),
            [cb { std::move(cb) }](const TemplateEngine::Parser &parser, const Filesystem::Path &outTarget) {
                parser.generateFiles(outTarget);
                if (cb)
                    cb(outTarget);
            });
    }

}
}
