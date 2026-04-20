#include "resourcestoolslib.h"

#include "resourcefile.h"

#include "Madgine_Tools/imguiicons.h"
#include "Madgine_Tools/renderer/imroot.h"
#include "imgui/imgui.h"
#include "imgui/imguiaddons.h"
#include "resourceeditor.h"

namespace Engine {
namespace Tools {

    MADGINE_TOOLS_EXPORT extern const ImGuiWindowClass windowClass;

    ResourceFileBase::ResourceFileBase(ResourceEditor &editor, Filesystem::Path path)
        : mEditor(editor)
        , mPath(std::move(path))
    {
    }

    bool ResourceFileBase::Begin(bool *open, ImGuiWindowFlags flags)
    {
        std::string fileName;
        if (!mPath.empty()) {
            fileName = mPath.filename().str();
        } else {
            fileName = "<unnamed>";
        }

        if (mHistory.isDirty())
            flags |= ImGuiWindowFlags_UnsavedDocument;

        ImGui::SetNextWindowDockID(mEditor.root().rootDockSpaceId(), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowClass(&windowClass);
        bool visible = mEditor.beginToolWindow(fileName.c_str(), open, flags | ImGuiWindowFlags_MenuBar);
        if (visible) {

            if (ImGui::BeginMenuBar()) {
                if (ImGui::BeginMenu("File")) {
                    if (ImGui::MenuItem(("Save '"s + mPath.filename().str() + "'").c_str(), "Ctrl+S", false, mHistory.isDirty() && !mPath.empty()))
                        save(mPath);
                    if (ImGui::MenuItem("Save as...")) {
                        mEditor.root().dialogs().show(
                            mEditor.resourceFilePicker(true), [this](const Filesystem::Path &p) { save(p); });
                    }
                    ImGui::EndMenu();
                }
                ImGui::EndMenuBar();
            }

            if (ImGui::BeginToolBar("Editor")) {

                if (!mHistory.isDirty())
                    ImGui::BeginDisabled();
                ImGui::SetNextItemShortcut(ImGuiKey_S | ImGuiMod_Ctrl);
                if (ImGui::Button(IMGUI_ICON_SAVE)) {
                    if (mPath.empty()) {
                        mEditor.root().dialogs().show(
                            mEditor.resourceFilePicker(true), [this](const Filesystem::Path &p) { save(p); });
                    } else {
                        save(mPath);
                    }
                }
                if (!mHistory.isDirty())
                    ImGui::EndDisabled();

                mHistory.renderControls();

                ImGui::EndToolBar();
            }
        }
        return visible;
    }

}
}