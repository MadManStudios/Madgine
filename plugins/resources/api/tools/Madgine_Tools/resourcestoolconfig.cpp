#include "resourcestoolslib.h"

#include "resourcestoolconfig.h"

#include "Meta/keyvalue/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

#include "Modules/uniquecomponent/uniquecomponentcollector.h"

#include "Madgine_Tools/inspector/inspector.h"

#include "Madgine_Tools/renderer/imroot.h"

#include "Madgine/resources/resourceloaderbase.h"
#include "Madgine/resources/resourcemanager.h"

#include "imgui/imguiaddons.h"

#include "imgui/imgui.h"

#include "Generic/projections.h"

#include "Meta/keyvalue/valuetype.h"
#include "Meta/keyvalue/scopefield.h"

namespace Engine {
namespace Tools {

    ResourcesToolConfig::ResourcesToolConfig(ImRoot &root)
        : Tool<ResourcesToolConfig>(root)
    {
    }

    Threading::Task<bool> ResourcesToolConfig::init()
    {
        Resources::ResourceManager &mgr = Resources::ResourceManager::getSingleton();

        Inspector &inspector = mRoot.getTool<Inspector>();

        for (std::unique_ptr<Resources::ResourceLoaderBase> &loader : mgr.mCollector) {
            for (const MetaTable *type : loader->resourceTypes()) {
                inspector.addPtrSuggestion(type, [&]() {
                    return loader->typedResources();
                });
            }
        }

        mResourceCache = mgr.buildResourceList();

        co_return true;
    }

    void ResourcesToolConfig::renderMenu()
    {
    }

    bool ResourcesToolConfig::renderConfiguration(const Filesystem::Path &config)
    {
        if (ImGui::CollapsingHeader("Resources")) {

            ImGui::Indent();

            if (ImGui::BeginTable("ResourceList", 2, ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY, { 0.0f, 200.0f })) {

                ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide);
                ImGui::TableSetupColumn("Path");
                ImGui::TableSetupScrollFreeze(0, 1);
                ImGui::TableHeadersRow();

                for (const auto &[path, resources] : mResourceCache) {
                    ImGui::PushID(path.c_str());

                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    auto it = mResourceConfig.find(path);
                    bool enabled = it != mResourceConfig.end();
                    if (ImGui::Checkbox("##enabled", &enabled)) {
                        if (enabled)
                            mResourceConfig.insert(path);
                        else
                            mResourceConfig.erase(it);
                    }

                    ImGui::TableNextColumn();
                    ImGui::Text(path);

                    ImGui::PopID();
                }
                ImGui::EndTable();
            }

            ImGui::Unindent();
        }

        return false;
    }

    void ResourcesToolConfig::loadConfiguration(const Filesystem::Path &config)
    {
        mResourceConfig.clear();
        std::ifstream in { config / "resources.list" };
        std::string line;
        while (std::getline(in, line))
            mResourceConfig.insert(line);
    }

    void ResourcesToolConfig::saveConfiguration(const Filesystem::Path &config)
    {
        std::ofstream out { config / "resources.list" };
        for (const Filesystem::Path &path : mResourceConfig)
            out << path << "\n";
    }

    std::string_view ResourcesToolConfig::key() const
    {
        return "ResourcesToolConfig";
    }

}
}

UNIQUECOMPONENT(Engine::Tools::ResourcesToolConfig);

METATABLE_BEGIN_BASE(Engine::Tools::ResourcesToolConfig, Engine::Tools::ToolBase)
METATABLE_END(Engine::Tools::ResourcesToolConfig)

SERIALIZETABLE_INHERIT_BEGIN(Engine::Tools::ResourcesToolConfig, Engine::Tools::ToolBase)
SERIALIZETABLE_END(Engine::Tools::ResourcesToolConfig)
