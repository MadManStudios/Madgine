#include "../launchertoolslib.h"

#include "launchertool.h"

#include "Platform/window/windowapi.h"

#include "Modules/uniquecomponent/uniquecomponentcollector.h"

#include "Madgine/serialize/filesystem/filemanager.h"
#include "Madgine/window/mainwindow.h"

#include "Meta/reflect/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

#include "Madgine_Tools/imgui/clientimroot.h"
#include "imgui/imgui.h"
#include "imgui/imguiaddons.h"

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

    void LauncherTool::renderMenu()
    {
    }

    bool LauncherTool::renderConfiguration(const Platform::Filesystem::Path &config)
    {
        bool changed = false;

        if (ImGui::CollapsingHeader("Launcher")) {
            ImGui::Indent();

            changed |= ImGui::InputText("Window Title", &mConfiguration["General"]["WINDOW_TITLE"]);

            ImGui::Unindent();
        }
        return changed;
    }

    void LauncherTool::loadConfiguration(const Platform::Filesystem::Path &config)
    {
        mConfiguration.loadFromDisk(config / "launcher.ini");
    }

    void LauncherTool::saveConfiguration(const Platform::Filesystem::Path &config)
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
