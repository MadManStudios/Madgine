#include "../templateslib.h"

#include "templates.h"

#include "Platform/filesystem/fsapi.h"

#include "Modules/plugins/plugin.h"
#include "Modules/uniquecomponent/uniquecomponentcollector.h"

#include "Meta/reflect/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

#include "Madgine_Tools/inspector/inspector.h"
#include "Madgine_Tools/pluginmanager/pluginmanager.h"
#include "Madgine_Tools/renderer/imroot.h"
#include "TemplateEngine/parser.h"
#include "imgui/imgui.h"
#include "imgui/imguiaddons.h"

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
#ifndef STATIC_BUILD
        if (ImGui::BeginMenu("Code")) {
            if (ImGui::BeginMenu("New")) {
                if (ImGui::MenuItem("UniqueComponent")) {
                    showUniqueComponentTemplateDialog();
                }
                ImGui::EndMenu();
            }
            ImGui::EndMenu();
        }

        if (ImGui::IsKeyDown(ImGuiMod_Ctrl)) {
            if (ImGui::BeginMenu("Dev")) {
                if (ImGui::BeginMenu("Templates")) {

                    for (Platform::Filesystem::FileQueryResult dir : Platform::Filesystem::listDirs(PROJECT_ROOT "/templates")) {
                        std::string name = dir.path().filename().str();
                        if (ImGui::MenuItem(name.c_str())) {
                            showTemplateDialog(name);
                        }
                    }

                    ImGui::EndMenu();
                }
                ImGui::EndMenu();
            }
        }
#endif
    }

    std::string_view Templates::key() const
    {
        return "Templates";
    }

#ifndef STATIC_BUILD
    void Templates::showTemplateDialog(std::string_view name, Closure<void(const Platform::Filesystem::Path &)> cb)
    {
        mRoot.dialogs().show(
            [](Templates *templates, std::string name) -> Dialog<TemplateEngine::Parser, Platform::Filesystem::Path> {
                Platform::Filesystem::Path outputPath { "C:\\Users\\Bub\\Desktop\\Test" };

                Platform::Filesystem::Path path = Platform::Filesystem::Path { PROJECT_ROOT "/templates" } / name;

                TemplateEngine::Parser parser { path };

                UndoStack history;

                DialogSettings &settings = co_await get_dialog_settings;
                settings.header = "Generate '" + name + "'";
                do {

                    if (ImGui::BeginTable("fields", 2, ImGuiTableFlags_Resizable)) {
                        for (auto &[key, value] : parser.fields()) {
                            TracedRoot root { history, value };
                            templates->mInspector->drawValue(key, root, true, value.type());
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
                        settings.open(templates->mRoot.directoryPicker(), outputPath);
                    }

                } while (co_yield settings);
                co_return { std::move(parser), std::move(outputPath) };
            }(this, std::string { name }),
            [cb { std::move(cb) }](const TemplateEngine::Parser &parser, const Platform::Filesystem::Path &outTarget) {
                parser.generateFiles(outTarget);
                if (cb)
                    cb(outTarget);
            });
    }

    void Templates::showUniqueComponentTemplateDialog(Closure<void(const Platform::Filesystem::Path &)> cb)
    {

        mRoot.dialogs().show(
            [](Templates *templates) -> Dialog<TemplateEngine::Parser, Platform::Filesystem::Path> {
                Platform::Filesystem::Path outputPath { "C:\\Users\\Bub\\Desktop\\Test" };

                Platform::Filesystem::Path path = PROJECT_ROOT "/templates/UniqueComponent";

                TemplateEngine::Parser parser { path };

                const Plugins::Plugin *currentPlugin = nullptr;
                const Plugins::RegistryBase *currentRegistry = nullptr;

                UndoStack history;

                DialogSettings &settings = co_await get_dialog_settings;
                settings.header = "Generate 'UniqueComponent'";
                do {

                    if (ImGui::BeginTable("fields", 2, ImGuiTableFlags_Resizable)) {
                        for (auto &[key, value] : parser.fields()) {
                            if (key == "prefix" || key == "base" || key == "precompiled" || key == "collector" || key == "plugin")
                                ImGui::BeginDisabled();
                            TracedRoot traced {history, value };                            
                            templates->mInspector->drawValue(key, traced, true, value.type());
                            if (key == "prefix" || key == "base" || key == "precompiled" || key == "collector" || key == "plugin")
                                ImGui::EndDisabled();
                        }
                        ImGui::EndTable();
                    }

                    if (PluginSelector("Plugin", currentPlugin)) {
                        outputPath = currentPlugin->info()->mSourceRoot;
                        parser.fields()["precompiled"] = std::string_view { currentPlugin->info()->mPrecompiledHeaderPath };
                        parser.fields()["plugin"] = currentPlugin->name();
                    }

                    if (ImGui::BeginCombo("Registry", currentRegistry ? currentRegistry->named_type_info().mFullName.data() : "")) {
                        for (Plugins::RegistryBase *registry : Plugins::registryRegistry()) {
                            if (ImGui::Selectable(registry->named_type_info().mFullName.data())) {
                                currentRegistry = registry;
                                std::string_view name = registry->named_type_info().mFullName;
                                assert(name.ends_with("Registry"));
                                parser.fields()["prefix"] = name.substr(0, name.size() - strlen("Registry"));

                                // parser.fields()["include"] =
                                parser.fields()["collector"] = std::string { Platform::Filesystem::Path { registry->mHeader() }.relative(currentRegistry->mBinary->mSourceRoot).str() };
                            }
                        }
                        ImGui::EndCombo();
                    }

                    if (!currentPlugin || !currentRegistry) {
                        ImGui::BeginDisabled();
                    }
                    ImGui::Text("Target:");
                    std::string s = outputPath;
                    if (ImGui::InputText("###Target", &s)) {
                        outputPath = s;
                        if (outputPath.isAbsolute()) {
                            outputPath = outputPath.relative(currentPlugin->info()->mSourceRoot);
                        }
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("...")) {
                        settings.open(templates->mRoot.directoryPicker(outputPath, outputPath, currentPlugin->info()->mSourceRoot), outputPath);
                    }
                    if (!currentPlugin || !currentRegistry) {
                        ImGui::EndDisabled();
                    }

                } while (co_yield settings);
                co_return { std::move(parser), std::move(outputPath) };
            }(this),
            [cb { std::move(cb) }](const TemplateEngine::Parser &parser, const Platform::Filesystem::Path &outTarget) {
                parser.generateFiles(outTarget);
                if (cb)
                    cb(outTarget);
            });
    }
#endif

}
}
