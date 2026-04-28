#include "../resourcestoolslib.h"

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
                        saveAs(mPath);
                    if (ImGui::MenuItem("Save as...")) {
                        mEditor.root().dialogs().show(
                            mEditor.resourceFilePicker(true), [this](const Filesystem::Path &p) { saveAs(p); });
                    }
                    ImGui::EndMenu();
                }
                ImGui::EndMenuBar();
            }

            if (ImGui::BeginToolBar("Editor")) {
                bool enabled = mHistory.isDirty();
                if (!enabled)
                    ImGui::BeginDisabled();
                ImGui::SetNextItemShortcut(ImGuiKey_S | ImGuiMod_Ctrl);
                if (ImGui::Button(IMGUI_ICON_SAVE)) {
                    save();
                }
                if (!enabled)
                    ImGui::EndDisabled();

                mHistory.renderControls();

                ImGui::EndToolBar();
            }
        }
        return visible;
    }

    void ResourceFileBase::save()
    {
        if (mPath.empty()) {
            mEditor.root().dialogs().show(
                mEditor.resourceFilePicker(true), [this](const Filesystem::Path &p) { saveAs(p); });
        } else {
            saveAs(mPath);
        }
    }

    Dialog<> ResourceFileBase::closeDialog()
    {
        DialogSettings &settings = co_await get_dialog_settings;

        settings.callbackOnDecline = true;

        if (mHistory.isDirty()) {            

            std::stringstream ss;
            ss << "File '" << mPath.filename() << "' has unsaved changes. Save?";

            do {
                ImGui::Text(ss.str());

            } while (co_yield settings);
            if (settings.accepted()) {
                save();
            }
        }

        co_return {};
    }


}
}