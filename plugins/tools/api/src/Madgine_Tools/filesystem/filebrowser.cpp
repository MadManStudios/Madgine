#include "../toolslib.h"

#include "filebrowser.h"

#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include "imgui/imguiaddons.h"

#include "Meta/keyvalue/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

#include "Interfaces/filesystem/fsapi.h"

#include "Modules/uniquecomponent/uniquecomponentcollector.h"

#include "../renderer/imroot.h"

UNIQUECOMPONENT(Engine::Tools::FileBrowser);

METATABLE_BEGIN_BASE(Engine::Tools::FileBrowser, Engine::Tools::ToolBase)
METATABLE_END(Engine::Tools::FileBrowser)

SERIALIZETABLE_INHERIT_BEGIN(Engine::Tools::FileBrowser, Engine::Tools::ToolBase)
SERIALIZETABLE_END(Engine::Tools::FileBrowser)

namespace Engine {
namespace Tools {

    FileBrowser::FileBrowser(ImRoot &root)
        : Tool<FileBrowser>(root)
    {
    }

    void FileBrowser::render()
    {
        if (ImGui::Begin("File Browser", &mVisible)) {
            ImGui::SetWindowDockingDir(mRoot.dockSpaceId(), ImGuiDir_Right, 0.2f, false, ImGuiCond_FirstUseEver);
            
            ImGui::FilePicker(mCurrentPath, mSelectedPath);
        }
        ImGui::End();
    }

    std::string_view FileBrowser::key() const
    {
        return "FileBrowser";
    }

}
}
