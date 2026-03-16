#include "../clienttoolslib.h"

#include "projectmanager.h"

#include "Generic/projections.h"

#include "Interfaces/filesystem/fsapi.h"
#include "Interfaces/window/windowapi.h"

#include "Modules/uniquecomponent/uniquecomponentcollector.h"

#include "Madgine/resources/resourcemanager.h"
#include "Madgine/window/layoutloader.h"
#include "Madgine/window/mainwindow.h"

#include "Meta/keyvalue/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

#include "Madgine_Tools/imguiicons.h"
#include "Madgine_Tools/templates/templates.h"
#include "clientimroot.h"
#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include "imgui/imguiaddons.h"

METATABLE_BEGIN_BASE(Engine::Tools::ProjectManager, Engine::Tools::ToolBase)
#ifndef MADGINE_MAINWINDOW_LAYOUT
    PROPERTY(Layout, layout, setLayout)
#endif
METATABLE_END(Engine::Tools::ProjectManager)

SERIALIZETABLE_INHERIT_BEGIN(Engine::Tools::ProjectManager, Engine::Tools::ToolBase)
#ifndef MADGINE_MAINWINDOW_LAYOUT
    ENCAPSULATED_FIELD(Layout, layoutString, setLayoutString)
    FIELD(mShowConfigurations)
    FIELD(mConfigs)
#endif
    FIELD(mShowSettings)
    FIELD(mShowTipsOnStartup)
SERIALIZETABLE_END(Engine::Tools::ProjectManager)

UNIQUECOMPONENT(Engine::Tools::ProjectManager)

namespace Engine {
namespace Tools {

    ProjectManager::ProjectManager(ImRoot &root)
        : Tool<ProjectManager>(root)
    {
        mVisible = true;
    }

    Threading::Task<bool> ProjectManager::init()
    {
        mWindow = &static_cast<const ClientImRoot &>(mRoot).window();

        mTemplates = &getTool<Templates>();

#ifndef MADGINE_MAINWINDOW_LAYOUT
        mWindow->taskQueue()->queue([this]() {
            load();
        });
#endif

        for (ToolBase *tool : mRoot.tools() | std::views::transform(projectionUniquePtrToPtr)) {
            if (tool->isEnabled()) {
                std::vector<Tip> tips = tool->tips();
                mTips.insert(mTips.end(), tips.begin(), tips.end());
            }
        }

        mTipIndex = std::rand() % mTips.size();

        co_return co_await ToolBase::init();
    }

    std::string_view ProjectManager::key() const
    {
        return "ProjectManager";
    }

#ifndef MADGINE_MAINWINDOW_LAYOUT
    void ProjectManager::renderLandingPage()
    {
        if (Window::LayoutLoader::getSingleton().resources().empty()) {

            if (beginGame()) {

                if (ImGui::Begin("GameOverlay")) {
                    ImVec2 size = ImGui::GetWindowContentRegionMax() - ImGui::GetWindowContentRegionMin();
                    size.y -= 20.0f;
                    ImGui::BeginVertical("vLanding", size);
                    ImGui::Spring();
                    ImGui::BeginHorizontal("aLanding");
                    ImGui::Text("Welcome to the Madgine!");
                    ImGui::EndHorizontal();
                    ImGui::Spring();
                    ImGui::BeginHorizontal("hLanding");

                    ImVec2 widget_size;
                    widget_size.x = floorf(ImGui::GetContentRegionAvail().x / 4);
                    widget_size.y = 150.0f;

                    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 2.0f);
                    ImGui::PushStyleColor(ImGuiCol_Button, { 0.2f, 0.2f, 0.2f, 1.0f });

                    ImGui::Spring();
                    if (ImGui::Button(IMGUI_ICON_FILE "\nCreate Project...", widget_size))
                        createProjectDialog();
                    ImGui::Spring();
                    ImGui::Button("Icon\nSomething", widget_size);
                    ImGui::Spring();

                    ImGui::PopStyleColor();
                    ImGui::PopStyleVar();

                    ImGui::EndHorizontal();
                    ImGui::Spring();
                    ImGui::BeginHorizontal("bLanding");
                    ImGui::Text("Below");
                    ImGui::EndHorizontal();
                    ImGui::Spring();
                    ImGui::EndVertical();
                }
                ImGui::End();
            }
            ImGui::End();
        }
    }

    void ProjectManager::renderConfigurations()
    {
        if (mShowConfigurations) {
            if (ImGui::Begin("Project Configurations", &mShowConfigurations, mUnsavedConfiguration ? ImGuiWindowFlags_UnsavedDocument : 0)) {

                bool openNewConfigPopup = false;

                if (ImGui::BeginCombo("##CurrentConfig", mCurrentConfig.c_str())) {

                    for (const Filesystem::Path &config : mConfigs)
                        if (ImGui::Selectable(config.c_str(), mCurrentConfig == config))
                            setCurrentConfig(config);

                    if (ImGui::Selectable("..."))
                        openNewConfigPopup = true;

                    ImGui::EndCombo();
                }

                if (ImGui::Button("Save")) {
                    for (ToolBase *tool : mRoot.tools() | std::views::transform(projectionUniquePtrToPtr)) {
                        if (tool->isEnabled())
                            tool->saveConfiguration(mCurrentConfig);
                    }
                    mUnsavedConfiguration = false;
                }

                if (!mCurrentConfig.empty()) {

                    for (ToolBase *tool : mRoot.tools() | std::views::transform(projectionUniquePtrToPtr)) {
                        if (tool->isEnabled())
                            mUnsavedConfiguration |= tool->renderConfiguration(mCurrentConfig);
                    }
                }

                if (openNewConfigPopup)
                    ImGui::OpenPopup("NewConfig");

                if (ImGui::BeginPopup("NewConfig")) {
                    if (ImGui::MenuItem("Create New")) {
                    }

                    if (ImGui::MenuItem("Open Existing")) {
                        mRoot.dialogs().show(
                            mRoot.directoryPicker(),
                            [this](const Filesystem::Path &path) {
                                setCurrentConfig(path);
                            });
                    }
                    if (ImGui::MenuItem("Copy Config")) {
                    }

                    ImGui::EndPopup();
                }
            }

            ImGui::End();
        }
    }

    void ProjectManager::renderGameMenu()
    {
        if (beginGame()) {
            if (ImGui::BeginMenuBar()) {
                if (ImGui::BeginMenu("Layout")) {
                    if (ImGui::MenuItem("New Layout...")) {
                        mRoot.dialogs().show([]() -> Dialog<std::string> {
                    DialogSettings &settings = co_await get_dialog_settings;
                    std::string layoutName;
                    do {
                        ImGui::InputText("Name", &layoutName);
                        settings.acceptPossible = !layoutName.empty();
                    } while (co_yield settings);
                    co_return layoutName; }(),
                            [this](const std::string &layoutName) {
                                // setLayout(layoutName); TODO Proper resource selection Dialog
                            });
                    }
                    if (ImGui::MenuItem("Save Layout")) {
                        save();
                    }

                    ImGui::Separator();

                    for (const auto &[name, res] : Window::LayoutLoader::getSingleton()) {
                        if (ImGui::MenuItem(name.data(), nullptr, mLayout == &res)) {
                            setLayout(Window::LayoutLoader::get(name));
                        }
                    }

                    ImGui::EndMenu();
                }
                ImGui::EndMenuBar();
            }
        }
        ImGui::End();
    }
#endif

    void ProjectManager::renderTips()
    {
        if (!mInitialized && ImGui::GetCurrentContext()->SettingsLoaded) {
            mShowTips = mShowTipsOnStartup;
            mInitialized = true;
        }

        if (mShowTips) {
            Tip &tip = mTips[mTipIndex];

            ImGui::SetNextWindowSize({ 500, 100 }, ImGuiCond_Always);
            std::string title = tip.mTitle + "###Tip";
            if (ImGui::Begin(title.c_str(), &mShowTips, ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoResize)) {
                ImGui::TextWrapped("%s", tip.mText.c_str());

                ImGui::BeginHorizontal("Controls", { 480, 0 });

                ImGui::Checkbox("Show Tips on Startup", &mShowTipsOnStartup);
                ImGui::Spring();
                if (ImGui::Button("Next Tip")) {
                    mTipIndex = (mTipIndex + 1) % mTips.size();
                }

                ImGui::EndHorizontal();
            }
            ImGui::End();
        }
    }

    void ProjectManager::renderSettingsPage()
    {
        if (mShowSettings) {
            if (ImGui::Begin("Settings", &mShowSettings)) {
                for (ToolBase *tool : mRoot.tools() | std::views::transform(projectionUniquePtrToPtr)) {
                    if (tool->isEnabled())
                        tool->renderSettings();
                }
            }
            ImGui::End();
        }
    }

    void ProjectManager::render()
    {
        renderSettingsPage();
        renderTips();
#ifndef MADGINE_MAINWINDOW_LAYOUT
        renderLandingPage();
        renderConfigurations();
        renderGameMenu();
#endif
    }

    void ProjectManager::renderMenu()
    {

        if (ImGui::BeginMenu("Project")) {
#ifndef MADGINE_MAINWINDOW_LAYOUT
            if (ImGui::MenuItem("New Project...")) {
                createProjectDialog();
            }

            ImGui::Separator();

            ImGui::MenuItem("Configurations", "", &mShowConfigurations);
#endif
            ImGui::MenuItem("Settings", "", &mShowSettings);

            ImGui::Separator();

            if (ImGui::MenuItem("Quit")) {
                mWindow->osWindow()->close();
            }

            ImGui::EndMenu();
        }
    }

    bool ProjectManager::renderConfiguration(const Filesystem::Path &config)
    {
        bool changed = false;
#ifndef MADGINE_MAINWINDOW_LAYOUT
        if (ImGui::CollapsingHeader("Client")) {
            ImGui::Indent();

            std::string layout = mConfiguration["General"]["LAYOUT"];
            if (ImGui::BeginCombo("Layout", layout.c_str())) {
                for (Resources::ResourceBase *res : Window::LayoutLoader::getSingleton().resources()) {
                    auto p = Resources::ResourceManager::getSingleton().makeRelative(res->path());
                    std::string relativePath = p.first + ":" + std::string { p.second.stem() };
                    if (ImGui::Selectable(relativePath.c_str(), relativePath == layout)) {
                        mConfiguration["General"]["LAYOUT"] = relativePath;
                        changed = true;
                    }
                }
                ImGui::EndCombo();
            }

            ImGui::Unindent();
        }
#endif
        return changed;
    }

    void ProjectManager::renderSettings()
    {
        ImGui::SeparatorText("Project Manager");

        ImGui::Checkbox("Show Tips on Startup", &mShowTipsOnStartup);
    }

#ifndef MADGINE_MAINWINDOW_LAYOUT
    void ProjectManager::loadConfiguration(const Filesystem::Path &config)
    {
        mConfiguration.loadFromDisk(config / "client.ini");
    }

    void ProjectManager::saveConfiguration(const Filesystem::Path &config)
    {
        mConfiguration.saveToDisk(config / "client.ini");
    }

    void ProjectManager::setCurrentConfig(const Filesystem::Path &config)
    {
        mCurrentConfig = config;
        mConfigs.insert(mCurrentConfig);
        for (ToolBase *tool : mRoot.tools() | std::views::transform(projectionUniquePtrToPtr)) {
            if (tool->isEnabled())
                tool->loadConfiguration(mCurrentConfig);
        }
    }

    void ProjectManager::createProjectDialog()
    {
        mTemplates->showTemplateDialog("NewProject", [this](const Filesystem::Path &path) {
            // TODO Show success message
        });
    }

    void ProjectManager::setLayout(Window::LayoutLoader::Resource *layout)
    {
        if (mLayout != layout) {
            mLayout = layout;
            load();
        }
    }

    Window::LayoutLoader::Resource *ProjectManager::layout() const
    {
        return mLayout;
    }

    std::string_view ProjectManager::layoutString() const
    {
        return mLayout ? mLayout->name() : "";
    }

    void ProjectManager::setLayoutString(std::string_view name)
    {
        setLayout(Window::LayoutLoader::get(name));
    }

    void ProjectManager::save()
    {
        mWindow->saveLayout(mLayout->path());
    }

    void ProjectManager::load()
    {
        if (mLayout) {
            mWindow->taskQueue()->queueTask(mWindow->loadLayout(mLayout));
        }
    }
#endif

    std::vector<Tip> ProjectManager::tips()
    {
        return {
            { "Documentation",
                "Need more information? Right click tools in Editor to access their documentation." }
        };
    }

}
}
