#include "../clienttoolslib.h"

#include "projectmanager.h"

#include "Meta/keyvalue/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

#include "imgui/imgui.h"
#include "imgui/imguiaddons.h"

#include "Interfaces/filesystem/fsapi.h"

#include "Madgine/resources/resourcemanager.h"
#include "Modules/uniquecomponent/uniquecomponentcollector.h"

#include "imgui/imgui_internal.h"

#include "Madgine_Tools/imguiicons.h"

#include "Madgine/window/mainwindow.h"
#include "clientimroot.h"

#include "Meta/serialize/hierarchy/statetransmissionflags.h"

#include "Interfaces/window/windowapi.h"

#include "Generic/projections.h"

#include "Madgine/window/layoutloader.h"

#include "Madgine_Tools/templates/templates.h"

METATABLE_BEGIN_BASE(Engine::Tools::ProjectManager, Engine::Tools::ToolBase)
PROPERTY(ProjectRoot, projectRootString, setProjectRoot)
PROPERTY(Layout, layout, setLayout)
METATABLE_END(Engine::Tools::ProjectManager)

SERIALIZETABLE_INHERIT_BEGIN(Engine::Tools::ProjectManager, Engine::Tools::ToolBase)
ENCAPSULATED_FIELD(ProjectRoot, projectRoot, setProjectRoot)
ENCAPSULATED_FIELD(Layout, layout, setLayout)
FIELD(mShowConfigurations)
FIELD(mShowSettings)
FIELD(mConfigs)
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

        mWindow->taskQueue()->queue([this]() {
            load();
        });

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

    void ProjectManager::renderLandingPage()
    {
        ImGuiDockNode *centralNode = ImGui::DockBuilderGetCentralNode(mRoot.rootDockSpaceId());
        if (mProjectRoot.empty() && centralNode->IsEmpty()) {

            ImGui::SetNextWindowPos(centralNode->Pos, ImGuiCond_Always);
            ImGui::SetNextWindowSize(centralNode->Size, ImGuiCond_Always);
            if (ImGui::Begin("Landing Page", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoResize)) {
                ImVec2 size = centralNode->Size;
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
                ImGui::PushStyleColor(ImGuiCol_Button, { 0.2f, 0.2f, 0.2f, 0.5f });

                ImGui::Spring();
                if (ImGui::Button(IMGUI_ICON_FILE "\nCreate Project...", widget_size))
                    createProjectDialog();
                ImGui::Spring();
                if (ImGui::Button(IMGUI_ICON_FOLDER "\nOpen Project...", widget_size))
                    openProjectDialog();
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
    }

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

    void ProjectManager::render()
    {
        renderConfigurations();
        renderSettingsPage();
        renderLandingPage();
        renderTips();
    }

    void ProjectManager::renderMenu()
    {

        if (ImGui::BeginMenu("Project")) {

            if (ImGui::MenuItem("New Project...")) {
                createProjectDialog();
            }

            if (ImGui::MenuItem("Open Project...")) {
                openProjectDialog();
            }

            ImGui::Separator();

            if (mProjectRoot.empty())
                ImGui::BeginDisabled();
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
                        setLayout(layoutName);
                    });
            }
            if (ImGui::MenuItem("Save Layout")) {
                save();
            }

            ImGui::Separator();

            for (const std::string &layout : projectLayouts()) {
                if (ImGui::MenuItem(layout.c_str(), nullptr, mLayout == layout)) {
                    setLayout(layout);
                }
            }
            if (mProjectRoot.empty())
                ImGui::EndDisabled();

            ImGui::Separator();

            ImGui::MenuItem("Configurations", "", &mShowConfigurations);
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

        if (ImGui::CollapsingHeader("Client")) {
            ImGui::Indent();

            std::string layout = mConfiguration["General"]["LAYOUT"];
            if (ImGui::BeginCombo("Layout", layout.c_str())) {
                for (Resources::ResourceBase *res : Window::LayoutLoader::getSingleton().resources()) {
                    if (ImGui::Selectable(res->name().data(), res->name() == layout)) {
                        mConfiguration["General"]["LAYOUT"] = res->name();
                        changed = true;
                    }
                }
                ImGui::EndCombo();
            }

            ImGui::Unindent();
        }
        return changed;
    }

    void ProjectManager::renderSettings()
    {
        ImGui::BeginGroupPanel("Project Manager");

        ImGui::Checkbox("Show Tips on Startup", &mShowTipsOnStartup);

        ImGui::EndGroupPanel();
    }

    void ProjectManager::loadConfiguration(const Filesystem::Path &config)
    {
        mConfiguration.loadFromDisk(config / "client.ini");
    }

    void ProjectManager::saveConfiguration(const Filesystem::Path &config)
    {
        mConfiguration.saveToDisk(config / "client.ini");
    }

    void ProjectManager::setProjectRoot(const Filesystem::Path &root)
    {
        if (mProjectRoot != root) {

            if (!mProjectRoot.empty()) {
                // Resources::ResourceManager::getSingleton().unregisterResourceLocation(mProjectRoot);
            }

            mProjectRoot = root;

            if (!mProjectRoot.empty()) {
                Resources::ResourceManager::getSingleton().registerResourceLocation(mProjectRoot / "data", 80);
            }

            const std::vector<std::string> &layouts = projectLayouts();
            mLayout = layouts.empty() ? "" : layouts.front();

            load();
        }
    }

    std::vector<std::string> ProjectManager::projectLayouts() const
    {
        if (mProjectRoot.empty())
            return {};
        std::vector<std::string> result;
        for (const Filesystem::Path &p : Filesystem::listFiles(mProjectRoot / "data")) {
            if (p.extension() == ".layout") {
                result.push_back(std::string { p.stem() });
            }
        }
        return result;
    }

    std::vector<Tip> ProjectManager::tips()
    {
        return {
            { "Documentation",
                "Need more information? Right click tools in Editor to access their documentation." }
        };
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
            setProjectRoot(path);
        });
    }

    void ProjectManager::openProjectDialog()
    {
        Filesystem::Path currentSelectionPath;
        if (!mProjectRoot.empty()) {
            currentSelectionPath = mProjectRoot.absolute();
        }

        mRoot.dialogs().show(
            mRoot.directoryPicker(currentSelectionPath),
            [this](const Filesystem::Path &selected) {
                setProjectRoot(selected);
            });
    }

    void ProjectManager::setLayout(const std::string &layout)
    {
        assert(!mProjectRoot.empty() || layout.empty());
        if (mLayout != layout) {
            mLayout = layout;
            load();
        }
    }

    const Filesystem::Path &ProjectManager::projectRoot() const
    {
        return mProjectRoot;
    }

    const std::string &ProjectManager::projectRootString() const
    {
        return projectRoot().str();
    }

    const std::string &ProjectManager::layout() const
    {
        return mLayout;
    }

    void ProjectManager::save()
    {
        Window::LayoutLoader::Resource *res = Window::LayoutLoader::get(mLayout);
        Filesystem::Path filePath;
        if (res) {
            filePath = res->path();
        } else {
            Filesystem::Path folder = mProjectRoot / "data";
            Filesystem::createDirectory(folder);
            filePath = folder / (mLayout + ".layout");
        }

        mWindow->saveLayout(filePath);
    }

    void ProjectManager::load()
    {
        if (!mLayout.empty()) {
            mWindow->loadLayout(mLayout);
        }
    }

}
}
