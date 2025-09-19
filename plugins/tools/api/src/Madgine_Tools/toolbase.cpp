#include "toolslib.h"

#include "toolbase.h"

#include "renderer/imroot.h"

#include "Meta/keyvalue/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

#include "imgui/imgui.h"
#include "imgui/imguiaddons.h"

#include "Interfaces/process/processapi.h"

namespace Engine {
namespace Tools {
    ToolBase::ToolBase(ImRoot &root, bool visible)
        : mVisible(visible)
        , mRoot(root)
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

    bool ToolBase::renderConfiguration(const Filesystem::Path &config)
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

    void ToolBase::loadConfiguration(const Filesystem::Path &config)
    {
    }

    void ToolBase::saveConfiguration(const Filesystem::Path &config)
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

    bool ToolBase::beginDefaultWindow(ImGuiDir dockingDir, const char *docTarget, ImGuiWindowFlags flags, const char *pluginSourceDir)
    {
        if (ImGui::Begin(key().data(), &mVisible, flags)) {

            float ratio = 0.2f;

            ImGui::SetWindowDockingDir(mRoot.dockSpaceId(), dockingDir, ratio, dockingDir == ImGuiDir_Down, ImGuiCond_FirstUseEver);

            if (docTarget) {
                if (ImGui::BeginPopupCompoundContextWindow()) {
                    if (ImGui::MenuItem("?")) {
                        Filesystem::Path path { "https://madmanstudios.github.io/Madgine/doc" };
                        Filesystem::Path pluginDir { pluginSourceDir };
                        path /= pluginDir.relative(SOURCE_DIR);
                        path /= "docs";
                        path /= docTarget;

                        Process::execute(path);
                    }
                    ImGui::EndPopup();
                }
            }
            return true;
        }
        return false;
    }
}
}

METATABLE_BEGIN(Engine::Tools::ToolBase)
PROPERTY(visible, isVisible, setVisible)
METATABLE_END(Engine::Tools::ToolBase)

SERIALIZETABLE_BEGIN(Engine::Tools::ToolBase)
FIELD(mVisible)
SERIALIZETABLE_END(Engine::Tools::ToolBase)
