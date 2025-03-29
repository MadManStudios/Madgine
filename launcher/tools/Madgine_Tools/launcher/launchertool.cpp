#include "../launchertoolslib.h"

#include "launchertool.h"

#include "imgui/imgui.h"
#include "imgui/imguiaddons.h"

#include "Meta/keyvalue/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

#include "Modules/uniquecomponent/uniquecomponentcollector.h"

#include "Madgine_Tools/imgui/clientimroot.h"

#include "Interfaces/window/windowapi.h"
#include "Madgine/window/mainwindow.h"

#include "Madgine/serialize/filesystem/filemanager.h"

UNIQUECOMPONENT(Engine::Tools::LauncherTool);

METATABLE_BEGIN_BASE(Engine::Tools::LauncherTool, Engine::Tools::ToolBase)
METATABLE_END(Engine::Tools::LauncherTool)

SERIALIZETABLE_INHERIT_BEGIN(Engine::Tools::LauncherTool, Engine::Tools::ToolBase)
SERIALIZETABLE_END(Engine::Tools::LauncherTool)

namespace Engine {
namespace Tools {

    LauncherTool::LauncherTool(ImRoot &root)
        : Tool<LauncherTool>(root)
    {
    }

    void LauncherTool::render()
    {
        if (ImGui::Begin("LauncherTool", &mVisible)) {
            std::string windowName = mMainWindow->osWindow()->title();
            if (ImGui::InputText("Window Title", &windowName))
                mMainWindow->osWindow()->setTitle(windowName.c_str());
        }
        ImGui::End();
    }

    bool LauncherTool::renderConfiguration(const Filesystem::Path &config)
    {
        bool changed = false;

        if (ImGui::CollapsingHeader("Launcher")) {
            ImGui::Indent();

            changed |= ImGui::InputText("Window Title", &mConfiguration["General"]["WINDOW_TITLE"]);

            ImGui::Unindent();
        }
        return changed;
    }

    void LauncherTool::loadConfiguration(const Filesystem::Path &config)
    {
        mConfiguration.loadFromDisk(config / "launcher.ini");
    }

    void LauncherTool::saveConfiguration(const Filesystem::Path &config)
    {
        mConfiguration.saveToDisk(config / "launcher.ini");
    }

    std::string_view LauncherTool::key() const
    {
        return "LauncherTool";
    }

    Threading::Task<bool> LauncherTool::init()
    {
        mMainWindow = &static_cast<ClientImRoot &>(mRoot).window();

        co_return co_await ToolBase::init();
    }

    Threading::Task<void> LauncherTool::finalize()
    {
        co_return co_await ToolBase::finalize();
    }

}
}
