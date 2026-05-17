#include "../toolslib.h"

#if ENABLE_PLUGINS

#    include "Generic/execution/algorithm.h"
#    include "Generic/execution/execution.h"

#    include "Interfaces/fetch/fetchapi.h"
#    include "Interfaces/filesystem/fsapi.h"

#    include "Modules/ini/inisection.h"
#    include "Modules/plugins/plugin.h"
#    include "Modules/plugins/pluginmanager.h"
#    include "Modules/plugins/pluginsection.h"
#    include "Modules/threading/taskqueue.h"
#    include "Modules/uniquecomponent/uniquecomponentcollector.h"

#    include "Meta/keyvalue/metatable_impl.h"
#    include "Meta/serialize/serializetable_impl.h"

#    include "../renderer/imroot.h"
#    include "imgui/imgui.h"
#    include "imgui/imgui_internal.h"
#    include "imgui/imguiaddons.h"
#    include "pluginmanager.h"

UNIQUECOMPONENT(Engine::Tools::PluginManager);

namespace Engine {
namespace Tools {

    PluginManager::PluginManager(ImRoot &root)
        : Tool<PluginManager>(root)
        , mManager(Plugins::PluginManager::getSingleton())
    {
    }

    void PluginManager::render()
    {
        ImGui::SetNextWindowSize({ 550, 400 }, ImGuiCond_FirstUseEver);
        if (beginToolPanel("Plugin Manager", &mVisible, ImGuiDir_Right)) {

            if (ImGui::BeginTabBar("Plugin settings")) {
                if (ImGui::BeginTabItem("Selection")) {

                    ImGui::TextColored(ImColor(255, 40, 40, 255), "Changes are only applied on restart!");

                    renderPluginSelection(false);

                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem("Hub")) {

                    ImGui::TextColored(ImColor(255, 40, 40, 255), "Changes are only applied on reconfigure/rebuild!");

                    bool fetch = mSources.empty() && !mFetching;

                    if (!mFetching)
                        ImGui::BeginDisabled();
                    fetch |= ImGui::Button("Refresh");
                    if (!mFetching)
                        ImGui::EndDisabled();

                    if (fetch) {
                        mFetching = true;
                        mLifetime.attach(FetchSender<JsonParser>("https://api.github.com/orgs/MadManStudios/repos", { "Accept: application/vnd.github+json", "User-Agent: Madgine" }) | Execution::then([this](JsonObject result) {
                            // LOG(result);
                            mSources.clear();
                            for (JsonObject &repo : result.asList()) {
                                if (!repo.asObject()["custom_properties"].asObject().contains("Madgine-Plugin-Group"))
                                    continue;
                                PluginSource &source = mSources.emplace_back();
                                source.mIcon = repo.asObject()["owner"].asObject()["avatar_url"].asString();
                                source.mName = repo.asObject()["name"].asString();
                                source.mUrl = repo.asObject()["clone_url"].asString();
                            }
                            mFetching = false;
                        }));
                    }

                    if (ImGui::BeginTable("Sources", 3, ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_SizingFixedFit)) {

                        ImGui::TableSetupColumn("Icon", 0);
                        ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch);
                        ImGui::TableSetupColumn("NoIdea", 0);

                        std::vector<std::string> dependencies = mCurrentDependencies;

                        for (PluginSource &source : mSources) {
                            ImGui::TableNextRow();
                            ImGui::TableNextColumn();
                            mRoot.Image(source.mIcon, { 64, 64 });
                            ImGui::TableNextColumn();
                            ImGui::Text(source.mName);
                            ImGui::TableNextColumn();

                            bool isActive = std::ranges::find(mCurrentDependencies, source.mUrl) != mCurrentDependencies.end();
                            std::erase(dependencies, source.mUrl);

                            ImGui::PushID(source.mName.c_str());
                            if (ImGui::Button(isActive ? "Deactivate##Button" : "Activate##Button")) {
                                if (isActive) {
                                    std::erase(mCurrentDependencies, source.mUrl);
                                } else {
                                    mCurrentDependencies.push_back(source.mUrl);
                                }
                                std::ofstream out { SOURCE_DIR "/dependencies.txt" };
                                for (const std::string &dep : mCurrentDependencies) {
                                    out << dep << '\n';
                                }
                            }
                            ImGui::PopID();
                        }

                        ImGui::TableNextRow();
                        ImGui::TableNextColumn();
                        ImGui::Dummy({ 0.0f, 20.0f });
                        ImGui::TableNextRow();
                        ImGui::TableNextColumn();
                        ImGui::Text("Custom");
                        ImGui::TableNextColumn();
                        ImGui::InputText("##CustomIn", &mCustomDependencyInput);
                        ImGui::TableNextColumn();
                        if (ImGui::Button("Add##CustomAdd")) {
                        }

                        auto locals = std::ranges::stable_partition(dependencies, [](const std::string &s) { return !s.starts_with("http"); });

                        for (const std::string &customDependency : std::ranges::subrange(dependencies.begin(), locals.begin())) {
                            Filesystem::Path path = customDependency;

                            std::string name { path.stem() };

                            ImGui::TableNextRow();
                            ImGui::TableNextColumn();
                            ImGui::Text(name);
                            ImGui::TableNextColumn();
                            ImGui::Text(customDependency);
                            ImGui::TableNextColumn();

                            ImGui::PushID(name.c_str());
                            if (ImGui::Button("Remove")) {
                                std::erase(mCurrentDependencies, customDependency);
                                std::ofstream out { SOURCE_DIR "/dependencies.txt" };
                                for (const std::string &dep : mCurrentDependencies) {
                                    out << dep << '\n';
                                }
                            }
                            ImGui::PopID();
                        }

                        ImGui::TableNextRow();
                        ImGui::TableNextColumn();
                        ImGui::Dummy({ 0.0f, 20.0f });
                        ImGui::TableNextRow();
                        ImGui::TableNextColumn();
                        ImGui::Text("Local");
                        ImGui::TableNextColumn();
                        std::string localDependency = mLocalDependencyInput.str();
                        if (ImGui::InputText("##LocalIn", &localDependency))
                            mLocalDependencyInput = localDependency;
                        ImGui::TableNextColumn();
                        if (ImGui::Button("...")) {
                        }
                        ImGui::SameLine();
                        if (ImGui::Button("Add##LocalAdd")) {
                        }

                        for (const std::string &localDependency : locals) {
                            Filesystem::Path path = localDependency;

                            std::string name { path.stem() };

                            ImGui::TableNextRow();
                            ImGui::TableNextColumn();
                            ImGui::Text(name);
                            ImGui::TableNextColumn();
                            ImGui::Text(localDependency);
                            ImGui::TableNextColumn();

                            ImGui::PushID(name.c_str());
                            if (ImGui::Button("Remove")) {
                                std::erase(mCurrentDependencies, localDependency);
                                std::ofstream out { SOURCE_DIR "/dependencies.txt" };
                                for (const std::string &dep : mCurrentDependencies) {
                                    out << dep << '\n';
                                }
                            }
                            ImGui::PopID();
                        }

                        ImGui::EndTable();
                    }

                    ImGui::EndTabItem();
                }
                ImGui::EndTabBar();
            }
        }
        ImGui::End();
    }

    bool PluginManager::renderConfiguration(const Filesystem::Path &config)
    {
        bool changed = false;

        if (ImGui::CollapsingHeader("Plugins")) {

            ImGui::Indent();

            changed |= renderPluginSelection(true);
            ImGui::Unindent();
        }

        return changed;
    }

    void PluginManager::loadConfiguration(const Filesystem::Path &config)
    {
        mCurrentConfiguration.loadFromDisk(config / "plugins.ini");
        for (auto &[sectionName, section] : mCurrentConfiguration) {
            if (!mManager.hasSection(sectionName))
                continue;
            Plugins::PluginSection &pSection = mManager[sectionName];
            for (auto &[pluginName, plugin] : section) {
                Plugins::Plugin *p = pSection.getPlugin(pluginName);
                if (p)
                    p->ensureModule(mManager);
            }
        }
    }

    void PluginManager::saveConfiguration(const Filesystem::Path &config)
    {
        mCurrentConfiguration.saveToDisk(config / "plugins.ini");
    }

    bool PluginManager::renderPluginSelection(bool isConfiguration)
    {
        bool changed = false;

        Ini::IniFile &file = isConfiguration ? mCurrentConfiguration : mManager.selection();

        for (auto &section : mManager) {
            if (ImGui::CollapsingHeader(section.name().c_str())) {

                if (isConfiguration) {
                    ImGui::Indent();
                }

                if (section.isExclusive() && !section.isAtleastOne()) {
                    bool noneLoaded = true;
                    for (auto &plugin : section) {
                        if (plugin.isLoaded(file)) {
                            noneLoaded = false;
                            break;
                        }
                    }
                    if (ImGui::RadioButton("<None>", noneLoaded)) {
                        section.unload(file);
                        changed = true;
                    }
                }

                for (auto &plugin : section) {
                    const std::string &project = plugin.project();

                    bool loaded = plugin.isLoaded(file);
                    plugin.ensureModule(mManager);

                    bool clicked = false;
                    std::string displayName { plugin.name() + " (" + project + ")" };
                    if (!isConfiguration) {
                        ImGui::BeginTreeArrow(&plugin);
                        ImGui::SameLine();
                    }
                    if (section.isExclusive()) {
                        clicked = ImGui::RadioButton(displayName.c_str(), loaded);
                        if (clicked)
                            loaded = true;
                    } else
                        clicked = ImGui::Checkbox(displayName.c_str(), &loaded);
                    if (clicked) {
                        changed = true;
                        if (loaded) {
                            section.loadPlugin(plugin.name(), file);
                        } else {
                            section.unloadPlugin(plugin.name(), file);
                        }
                    }
                    if (!isConfiguration && ImGui::EndTreeArrow()) {
                        const Plugins::BinaryInfo *binInfo = plugin.info();

                        const char **dep = binInfo->mPluginDependencies;
                        if (*dep && ImGui::TreeNode("Dependencies")) {
                            while (*dep) {
                                ImGui::Text("%s", *dep);
                                ++dep;
                            }
                            ImGui::TreePop();
                        }

                        if (ImGui::TreeNode("UniqueComponents")) {
                            for (UniqueComponent::RegistryBase *reg : UniqueComponent::registryRegistry()) {
                                for (UniqueComponent::CollectorInfoBase *info : *reg) {
                                    if (info->mBinary == binInfo && ImGui::TreeNode(reg->type_info().type_name().data(), "%.*s", static_cast<int>(reg->type_info().type_name().size()), reg->type_info().type_name().data())) {
                                        for (const std::pair<std::vector<UniqueComponent::TypeInfo>, UniqueComponent::TypeInfo> &components : info->mElementInfos) {
                                            ImGui::Text(components.first.front().type_name());
                                        }
                                        ImGui::TreePop();
                                    }
                                }
                            }
                            ImGui::TreePop();
                        }
                        ImGui::TreePop();
                    }
                }
                if (isConfiguration)
                    ImGui::Unindent();
            }
        }

        return changed;
    }

    Threading::Task<bool> PluginManager::init()
    {
        if (!co_await ToolBase::init())
            co_return false;

        mRoot.taskQueue()->queue([this]() -> Threading::ImmediateTask<void> {
            co_await mLifetime;
        });

        if (Filesystem::exists(SOURCE_DIR "/dependencies.txt")) {
            std::ifstream in { SOURCE_DIR "/dependencies.txt" };
            std::string line;
            while (std::getline(in, line)) {
                mCurrentDependencies.push_back(std::move(line));
            }
        }

        co_return true;
    }

    Threading::Task<void> PluginManager::finalize()
    {
        co_await ToolBase::finalize();
    }

    std::string_view PluginManager::key() const
    {
        return "Plugin Manager";
    }

    bool PluginSelector(const char *label, const Plugins::Plugin *&outPlugin, bool requiresData)
    {
        bool changed = false;

        if (ImGui::BeginCombo(label, outPlugin ? outPlugin->name().c_str() : "")) {

            Plugins::PluginManager &manager = Plugins::PluginManager::getSingleton();
            for (auto &section : manager) {
                for (auto &plugin : section) {
                    plugin.ensureModule(manager);
                    if (requiresData && plugin.info()->mDataPath.empty())
                        continue;
                    if (plugin.isLoaded(Plugins::PluginManager::getSingleton().selection())) {
                        if (ImGui::Selectable(plugin.name().c_str(), &plugin == outPlugin)) {
                            outPlugin = &plugin;
                            changed = true;
                        }
                    }
                }
            }

            ImGui::EndCombo();
        }

        return changed;
    }

}
}

METATABLE_BEGIN_BASE(Engine::Tools::PluginManager, Engine::Tools::ToolBase)
METATABLE_END(Engine::Tools::PluginManager)

SERIALIZETABLE_INHERIT_BEGIN(Engine::Tools::PluginManager, Engine::Tools::ToolBase)
SERIALIZETABLE_END(Engine::Tools::PluginManager)

#endif