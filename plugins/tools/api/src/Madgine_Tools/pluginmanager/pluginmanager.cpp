#include "../toolslib.h"

#if ENABLE_PLUGINS

#    include "pluginmanager.h"

#    include "imgui/imgui.h"
#    include "imgui/imgui_internal.h"

#    include "Modules/plugins/pluginmanager.h"

#    include "Meta/keyvalue/metatable_impl.h"
#    include "Meta/serialize/serializetable_impl.h"

#    include "Interfaces/filesystem/fsapi.h"

#    include "Modules/plugins/plugin.h"
#    include "Modules/plugins/pluginsection.h"

#    include "Modules/uniquecomponent/uniquecomponentcollector.h"

#    include "imgui/imguiaddons.h"

#    include "Modules/ini/inisection.h"

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
        if (ImGui::Begin("Plugin Manager", &mVisible)) {

            ImGui::TextColored(ImColor(255, 40, 40, 255), "Changes are only applied on restart!");

            renderPluginSelection(false);
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

        for (auto &section : mManager) {
            if (ImGui::CollapsingHeader(section.name().c_str())) {
                Ini::IniFile &file = isConfiguration ? mCurrentConfiguration : mManager.selection();
                if (isConfiguration) {
                    ImGui::Indent();
                }
                for (auto &plugin : section) {
                    const std::string &project = plugin.project();

                    bool loaded = plugin.isLoaded(file);

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
                        if (loaded) {
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
                                        if (info->mBinary == binInfo && ImGui::TreeNode(info->mBaseInfo->mTypeName.data(), "%.*s", static_cast<int>(info->mBaseInfo->mTypeName.size()), info->mBaseInfo->mTypeName.data())) {
                                            for (const std::pair<std::vector<const TypeInfo *>, const TypeInfo*> &components : info->mElementInfos) {
                                                ImGui::Text(components.first.front()->mTypeName);
                                            }
                                            ImGui::TreePop();
                                        }
                                    }
                                }
                                ImGui::TreePop();
                            }
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
        co_return co_await ToolBase::init();
    }

    std::string_view PluginManager::key() const
    {
        return "Plugin Manager";
    }

}
}

METATABLE_BEGIN_BASE(Engine::Tools::PluginManager, Engine::Tools::ToolBase)
METATABLE_END(Engine::Tools::PluginManager)

SERIALIZETABLE_INHERIT_BEGIN(Engine::Tools::PluginManager, Engine::Tools::ToolBase)
SERIALIZETABLE_END(Engine::Tools::PluginManager)

#endif