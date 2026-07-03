#include "toolslib.h"

#include "toolbase.h"

#include "Platform/process/processapi.h"

#include "Meta/reflect/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include "imgui/imguiaddons.h"
#include "renderer/imroot.h"

namespace Engine {
namespace Tools {

    MADGINE_TOOLS_EXPORT extern const ImGuiWindowClass windowClass = []() {
        ImGuiWindowClass topLevelClass;
        topLevelClass.ClassId = 1;
        topLevelClass.ViewportFlagsOverrideSet = ImGuiViewportFlags_NoAutoMerge;
        topLevelClass.ViewportFlagsOverrideClear = ImGuiViewportFlags_NoDecoration | ImGuiViewportFlags_NoTaskBarIcon;
        topLevelClass.ParentViewportId = 0; // Top level window
        topLevelClass.DockingAllowUnclassed = false;
        topLevelClass.DockingAlwaysTabBar = true;

        return topLevelClass;
    }();

    ToolBase::ToolBase(ImRoot &root)
        : mRoot(root)
    {
    }

    void ToolBase::render()
    {
    }

    void ToolBase::renderMenu()
    {
        if (ImGui::BeginMenu("Views")) {
            ImGui::MenuItem(key().data(), "", &mVisible);
            ImGui::EndMenu();
        }
    }

    bool ToolBase::renderConfiguration(const Platform::Filesystem::Path &config)
    {
        return false;
    }

    void ToolBase::renderSettings()
    {
    }

    void ToolBase::renderMetrics()
    {
    }

    void ToolBase::update()
    {
        if (mVisible)
            render();
    }

    void ToolBase::loadConfiguration(const Platform::Filesystem::Path &config)
    {
    }

    void ToolBase::saveConfiguration(const Platform::Filesystem::Path &config)
    {
    }

    std::vector<Tip> ToolBase::tips()
    {
        return {};
    }

    bool ToolBase::isVisible() const
    {
        return mVisible;
    }

    void ToolBase::setVisible(bool v)
    {
        mVisible = v;
    }

    bool ToolBase::isEnabled() const
    {
        return mEnabled;
    }

    void ToolBase::setEnabled(bool e)
    {
        mEnabled = e;
    }

    ToolBase &ToolBase::getTool(size_t index)
    {
        return mRoot.getTool(index);
    }

    ImRoot &ToolBase::root()
    {
        return mRoot;
    }

    Threading::TaskQueue *ToolBase::taskQueue() const
    {
        return mRoot.taskQueue();
    }

    Threading::Task<bool> ToolBase::init()
    {
        co_return true;
    }

    Threading::Task<void> ToolBase::finalize()
    {
        co_return;
    }

    bool ToolBase::beginTool(const char *name, bool *open, ImGuiDir dockingDir, ImGuiWindowFlags flags, const char *docTarget, const char *pluginSourceDir)
    {
        ImGui::SetNextWindowClass(&windowClass);
        if (dockingDir == ImGuiDir_None)
            ImGui::SetNextWindowDockID(ImGui::DockBuilderGetCentralNode(mRoot.rootDockSpaceId())->ID, ImGuiCond_FirstUseEver);

        std::string fullName = std::string { name } + "##" + std::string { key() };

        if (ImGui::Begin(fullName.c_str(), open, flags)) {

            float ratio = 0.2f;

            if (dockingDir != ImGuiDir_None)
                ImGui::SetWindowDockingDir(mRoot.rootDockSpaceId(), dockingDir, ratio, dockingDir == ImGuiDir_Down, ImGuiCond_FirstUseEver);

            if (docTarget) {
                if ((flags & ImGuiWindowFlags_MenuBar) ? ImGui::BeginMenuBar() : ImGui::BeginPopupCompoundContextWindow()) {
                    if (ImGui::MenuItem("?")) {
                        Platform::Filesystem::Path path { "https://madmanstudios.github.io/Madgine/doc" };
                        Platform::Filesystem::Path pluginDir { pluginSourceDir };
                        path /= pluginDir.relative(SOURCE_DIR);
                        path /= "docs";
                        path /= docTarget;

                        Platform::Process::execute(path);
                    }
                    (flags & ImGuiWindowFlags_MenuBar) ? ImGui::EndMenuBar() : ImGui::EndPopup();
                }
            }

            return true;
        }
        return false;
    }

    bool ToolBase::beginToolWindow(const char *name, bool *open, ImGuiWindowFlags flags, const char *docTarget, const char *pluginSourceDir)
    {
        bool visible = beginTool(name, open, ImGuiDir_None, flags, docTarget, pluginSourceDir);

        if (ImGui::BeginToolBar("Dummy")) {
            ImGui::EndToolBar();
        }

        ImGuiID dockId = ImGui::GetWindowDockID();
        if (dockId == 0)
            dockId = ImGui::GetID("Floating");
        mDockSpaceId = ImHashStr(key().data(), 0, dockId);
        mCurrentWindowId = ImGui::GetCurrentWindow()->ID;
        ImGuiID lastDockId = (ImGuiID)std::exchange(*ImGui::GetStateStorage()->GetIntRef(ImGui::GetID("DockID")), (int)mDockSpaceId);
        if (lastDockId != 0 && lastDockId != mDockSpaceId) {
            ImGuiDockNode *node = ImGui::DockBuilderGetNode(mDockSpaceId);
            ImGuiDockNode *prevNode = ImGui::DockBuilderGetNode(lastDockId);
            if (prevNode) {
                if (!node) {
                    // new node
                    ImVector<const char *> remapping;
                    std::vector<std::string> names;
                    ImGuiContext &g = *GImGui;
                    for (ImGuiWindow *window : g.Windows) {
                        std::string name = window->Name;

                        auto pos = name.find("##");
                        if (pos == std::string::npos)
                            continue;

                        std::string trimmed = name.substr(0, pos);
                        if (name != std::format("{}##{:x}", trimmed, lastDockId))
                            continue;

                        names.push_back(name);
                        names.push_back(std::format("{}##{:x}", trimmed, mDockSpaceId));
                    }
                    for (const std::string &name : names) {
                        remapping.push_back(name.c_str());
                    }
                    ImGui::DockBuilderCopyDockSpace(lastDockId, mDockSpaceId, &remapping);
                    ImGui::DockBuilderFinish(mDockSpaceId);
                }
            }

            mRoot.mDockSpaces.try_emplace(lastDockId, 0);
        }

        ++mRoot.mDockSpaces.try_emplace(mDockSpaceId, 0).first->second;

        if (ImGui::GetCurrentWindow()->Hidden)
            visible = false;

        if (visible) {
            ImGui::DockSpace(mDockSpaceId);

            return true;
        }

        return false;
    }

    bool ToolBase::beginToolPanel(const char *name, bool *open, ImGuiDir dockingDir, ImGuiWindowFlags flags, const char *docTarget, const char *pluginSourceDir)
    {
        return beginTool(name, open, dockingDir, flags, docTarget, pluginSourceDir);
    }

    bool ToolBase::beginSubPanel(const char *name, bool *open, ImGuiDir dockingDir, float ratio, ImGuiWindowFlags flags)
    {
        assert(dockingDir != ImGuiDir_None);

        ImGuiWindowClass subPanelClass;
        subPanelClass.FocusRouteParentWindowId = mCurrentWindowId;
        ImGui::SetNextWindowClass(&subPanelClass);

        std::string newName = std::format("{}##{:x}", name, mDockSpaceId);
        if (ImGui::Begin(newName.c_str(), open, flags)) {
            ImGui::SetWindowDockingDir(mDockSpaceId, dockingDir, ratio, false, ImGuiCond_FirstUseEver);
            return true;
        }
        return false;
    }

    bool ToolBase::beginGame()
    {
        if (ImGui::Begin("Game")) {
            mDockSpaceId = mRoot.gameDockSpaceId();
            return true;
        }
        return false;
    }

    bool ToolBase::beginContent(ImGuiWindowFlags flags)
    {
        std::string name = std::format("Content##{:x}", mDockSpaceId);

        ImGuiWindowClass window_class;
        window_class.DockNodeFlagsOverrideSet = ImGuiDockNodeFlags_NoTabBar;
        window_class.FocusRouteParentWindowId = mCurrentWindowId;
        ImGui::SetNextWindowClass(&window_class);

        ImGui::SetNextWindowDockID(ImGui::DockBuilderGetCentralNode(mDockSpaceId)->ID, ImGuiCond_Appearing);
        return ImGui::Begin(name.c_str(), nullptr, flags | ImGuiWindowFlags_NoMove);
    }

    void ToolBase::focusToolWindow(const char *name)
    {
        std::string fullName = std::string { name } + "##" + std::string { key() };
        ImGui::SetWindowFocus(fullName.c_str());
    }

    ImGuiID ToolBase::dockSpaceId() const
    {
        return mDockSpaceId;
    }

    Dialog<> ToolBase::closeDialog()
    {
        co_return {};
    }

}
}

METATABLE_BEGIN(Engine::Tools::ToolBase)
    PROPERTY(visible, isVisible, setVisible)
METATABLE_END(Engine::Tools::ToolBase)

SERIALIZETABLE_BEGIN(Engine::Tools::ToolBase)
    FIELD(mVisible)
SERIALIZETABLE_END(Engine::Tools::ToolBase)
