#include "../toolslib.h"

#include "dialogs.h"

#include "imgui/imgui.h"

namespace Engine {
namespace Tools {

    void DialogContainer::show(CoroutineHandle<DialogPromise> dialog)
    {
        dialog->mTargetContainer = this;
        mDialogs.push_back(std::move(dialog));
    }

    void DialogContainer::render()
    {
        std::vector<CoroutineHandle<DialogPromise>> dialogs = std::move(mDialogs);

        for (CoroutineHandle<DialogPromise> &dialog : dialogs) {
            if (renderHeader()) {
                DialogSettings settings;
                dialog->mOutSettings = &settings;
                CoroutineHandle<DialogPromise> continuation;
                dialog->mOutHandle = &continuation;
                dialog.release().resume();
                std::optional<DialogResult> done = renderFooter(settings);
                if (continuation) {
                    if (done) {
                        if (*done != DialogResult::Canceled) {
                            continuation->mDone = *done;
                            continuation.release().resume();
                        }
                    } else {
                        mDialogs.push_back(std::move(continuation));
                    }
                }
            }
        }
    }

    bool DialogContainer::renderHeader()
    {
        std::string header = " "; // mSettings.header.empty() ? " " : mSettings.header;

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

    std::optional<DialogResult> DialogContainer::renderFooter(DialogSettings settings)
    {
        ImGui::BeginHorizontal("Buttons");

        ImGui::Spring();

        std::optional<DialogResult> result;

        if (settings.showAccept) {
            if (!settings.acceptPossible)
                ImGui::BeginDisabled();
            if (ImGui::Button(settings.acceptText.c_str())) {
                result = DialogResult::Accepted;
            }
            if (!settings.acceptPossible)
                ImGui::EndDisabled();
        }

        if (settings.showDecline) {
            if (ImGui::Button(settings.declineText.c_str())) {
                result = DialogResult::Declined;
            }
        }

        if (settings.showCancel) {
            if (ImGui::Button(settings.cancelText.c_str())) {
                result = DialogResult::Canceled;
            }
        }

        ImGui::EndHorizontal();
        ImGui::EndVertical();

        ImGui::EndPopup();
        ImGui::PopID();

        return result;
    }

}
}
