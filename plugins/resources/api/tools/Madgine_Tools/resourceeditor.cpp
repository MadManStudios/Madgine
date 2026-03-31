#include "resourcestoolslib.h"

#include "resourceeditor.h"

#include "Interfaces/filesystem/fsapi.h"

#include "Modules/plugins/plugin.h"

#include "Madgine/resources/resourceloaderbase.h"
#include "Madgine/resources/resourcemanager.h"

#include "Madgine_Tools/pluginmanager/pluginmanager.h"
#include "Madgine_Tools/renderer/imroot.h"
#include "imgui/imgui_internal.h"
#include "imgui/imguiaddons.h"
#include "resourcestool.h"

namespace Engine {
namespace Tools {

    MADGINE_TOOLS_EXPORT extern const ImGuiWindowClass windowClass;

    ResourceEditor::ResourceEditor(ImRoot &root)
        : ToolBase(root)
    {
        mVisible = true;
    }

    void ResourceEditor::renderMenu()
    {
        if (ImGui::BeginMenu("Resources")) {

            if (ImGui::BeginMenu("New...")) {
                if (ImGui::MenuItem(mType.c_str())) {
                    open(nullptr);
                }
                ImGui::EndMenu();
            }
            /* if (ImGui::MenuItem("Open Graph")) {
                mRoot.dialogs().show(
                    []() -> Dialog<std::string> {
                        std::string selection;
                        bool alreadyClicked = false;

                        DialogSettings &settings = co_await get_dialog_settings;
                        settings.acceptText = "Open";

                        do {
                            ImGui::BeginChild("GraphList", { 0.0f, -ImGui::GetFrameHeightWithSpacing() });

                            for (const std::pair<std::string_view, ScopePtr> &res : NodeGraph::NodeGraphLoader::getSingleton().typedResources()) {

                                bool selected = selection == res.first;

                                if (ImGui::Selectable(res.first.data(), selected)) {
                                    selection = res.first;
                                }

                                if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {
                                    selection = res.first;
                                    alreadyClicked = true;
                                }
                            }

                            ImGui::EndChild();

                            settings.acceptPossible = !selection.empty();
                        } while (!alreadyClicked && (co_yield settings));

                        co_return selection;
                    }(),
                    [this](const std::string &selection) {
                        load(selection);
                    });
            }

            if (ImGui::MenuItem("Save Graph", "", false)) {
                if (mFilePath.empty()) {
                    mRoot.dialogs().show(
                        mRoot.filePicker(true),
                        [this](const Filesystem::Path &path) {
                            mFilePath = path;
                            save();
                        });
                } else {
                    save();
                }
            }*/
            ImGui::EndMenu();
        }
    }

    Threading::Task<bool> ResourceEditor::init(Resources::ResourceLoaderBase &loader, std::string type)
    {
        if (!co_await ToolBase::init())
            co_return false;

        mResourceLoader = &loader;
        mType = std::move(type);

        mManager = &getTool<ResourcesTool>();
        mManager->registerEditor(&loader, this);

        co_return true;
    }

    bool ResourceEditor::BeginResourceFile(const void *id, const Filesystem::Path &path, bool isDirty, Closure<void(const Filesystem::Path &)> save, bool *open, ImGuiWindowFlags flags)
    {
        std::string fileName;
        if (!path.empty()) {
            fileName = path.filename().str();
        } else {
            fileName = "<unnamed>";
        }

        if (isDirty)
            flags |= ImGuiWindowFlags_UnsavedDocument;

        ImGui::SetNextWindowDockID(mRoot.rootDockSpaceId(), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowClass(&windowClass);
        bool visible = beginToolWindow(fileName.c_str(), open, flags | ImGuiWindowFlags_MenuBar);
        if (visible) {

            if (ImGui::BeginMenuBar()) {
                if (ImGui::BeginMenu("File")) {
                    if (ImGui::MenuItem(("Save '"s + path.filename().str() + "'").c_str(), "Ctrl+S", false, isDirty && !path.empty()))
                        save(path);
                    if (ImGui::MenuItem("Save as...")) {
                        mRoot.dialogs().show(
                            resourceFilePicker(true), std::move(save));
                    }
                    ImGui::EndMenu();
                }
                ImGui::EndMenuBar();
            }

            if (beginToolBar("Editor")) {

                if (!isDirty)
                    ImGui::BeginDisabled();
                if (ImGui::Button("Save")) {
                    if (path.empty()) {
                        mRoot.dialogs().show(
                            resourceFilePicker(true), std::move(save));
                    } else {
                        save(path);
                    }
                }
                if (!isDirty)
                    ImGui::EndDisabled();

                endToolBar();
            }

            if (ImGui::Shortcut(ImGuiKey_S | ImGuiMod_Ctrl)) {
                if (path.empty())
                    mRoot.dialogs().show(
                        resourceFilePicker(true), std::move(save));
                else
                    save(path);
            }
        }
        return visible;
    }

    Dialog<Filesystem::Path> ResourceEditor::resourceFilePicker(bool allowNewFile, Filesystem::Path path, Filesystem::Path selected)
    {
        DialogSettings &settings = co_await get_dialog_settings;
        settings.acceptText = allowNewFile ? "Save" : "Open";
        settings.declineText = "Cancel";

        const Plugins::Plugin *plugin = nullptr;

        ImGui::FilesystemPickerOptions options;
        options.mExtensions = mResourceLoader->fileExtensions();

        bool implicitlyAccepted = false;
        do {
            

            bool enabled = true;

            #if ENABLE_PLUGINS
            if (PluginSelector("Plugin", plugin, true)) {
                selected = Filesystem::Path { SOURCE_DIR } / plugin->info()->mDataPath;
            }

            if (plugin) {
                options.mBase = Filesystem::Path { SOURCE_DIR } / plugin->info()->mDataPath;
            } else {
                enabled = false;
            }
            #endif

            if (!enabled)
                ImGui::BeginDisabled();
            ImGui::FilePicker(path, selected, &implicitlyAccepted, options);
            if (!enabled)
                ImGui::EndDisabled();

            settings.acceptPossible = enabled && !selected.empty() && (allowNewFile || Filesystem::exists(selected));
        } while (!implicitlyAccepted && (co_yield settings));
        co_return selected;
    }

}
}