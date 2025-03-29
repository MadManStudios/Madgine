#include "../toolslib.h"

#include "dialogs.h"

#include "imgui/imgui.h"

namespace Engine {
namespace Tools {

    void DialogSettings::accept() {
        result = DialogResult::Accepted;
    }

    void DialogSettings::decline()
    {
        result = DialogResult::Declined;
    }

    void DialogSettings::cancel()
    {
        result = DialogResult::Canceled;
    }

    void DialogContainer::show(CoroutineHandle<DialogPromise> dialog)
    {
        dialog->mTargetContainer = this;
        mDialogs.push_back(std::move(dialog));
    }

    void DialogContainer::render()
    {
        std::vector<CoroutineHandle<DialogPromise>> dialogs = std::move(mDialogs);

        for (CoroutineHandle<DialogPromise> &dialog : dialogs) {
            if (renderHeader(dialog->mSettings)) {                
                CoroutineHandle<DialogPromise> continuation;
                dialog->mOutHandle = &continuation;
                DialogSettings backup = dialog->mSettings;
                dialog.release().resume();
                DialogSettings &settings = continuation ? continuation->mSettings : backup;
                renderFooter(settings);
                if (continuation) {       
                    if (settings.result) {
                        if (*settings.result != DialogResult::Canceled) {                            
                            continuation.release().resume();
                        }
                    } else {
                        mDialogs.push_back(std::move(continuation));
                    }
                }

            }
        }
    }

    bool DialogContainer::renderHeader(DialogSettings &settings)
    {
        std::string header = settings.header;

        ImGui::PushID(this);
        ImGui::OpenPopup(header.c_str());

        ImGui::SetNextWindowSize({ 500, 400 }, ImGuiCond_FirstUseEver);
        if (ImGui::BeginPopupModal(header.c_str())) {

            ImGui::BeginVertical("Main");

            return true;
        } else {
            ImGui::PopID();
            return false;
        }
    }

    void DialogContainer::renderFooter(DialogSettings &settings)
    {
        ImGui::BeginHorizontal("Buttons");

        ImGui::Spring();

        if (settings.showAccept) {
            if (!settings.acceptPossible)
                ImGui::BeginDisabled();
            if (ImGui::Button(settings.acceptText.c_str())) {
                settings.accept();
            }
            if (!settings.acceptPossible)
                ImGui::EndDisabled();
        }

        if (settings.showDecline) {
            if (ImGui::Button(settings.declineText.c_str())) {
                settings.decline();
            }
        }

        if (settings.showCancel) {
            if (ImGui::Button(settings.cancelText.c_str())) {
                settings.cancel();
            }
        }

        ImGui::EndHorizontal();
        ImGui::EndVertical();

        ImGui::EndPopup();
        ImGui::PopID();
    }

}
}
