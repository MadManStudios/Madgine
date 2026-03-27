#include "../toolslib.h"

#include "dialogs.h"

#include "imgui/imgui.h"

namespace Engine {
namespace Tools {

    void DialogSettings::accept()
    {
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

    void DialogSettings::open(CoroutineHandle<DialogPromise> handle)
    {
        mSubDialogs.push_back(std::move(handle));
    }

    void DialogContainer::show(CoroutineHandle<DialogPromise> dialog)
    {
        mDialogs.push_back(std::move(dialog));
    }

    void DialogContainer::render()
    {
        handleDialogs(mDialogs);
    }

    void DialogContainer::handleDialogs(std::vector<CoroutineHandle<DialogPromise>> &dialogs)
    {
        std::vector<CoroutineHandle<DialogPromise>> localDialogs = std::move(dialogs);

        for (CoroutineHandle<DialogPromise> &dialog : localDialogs) {
            if (renderHeader(dialog->mSettings)) {
                dialog.resume();
                handleDialogs(dialog->mSettings.mSubDialogs);
                renderFooter(dialog->mSettings);
                if (dialog->mSettings.result) {
                    if (*dialog->mSettings.result != DialogResult::Canceled && !dialog.done()) {
                        dialog.resume();
                    }
                    assert(*dialog->mSettings.result == DialogResult::Canceled || dialog.done());
                } else {
                    dialogs.push_back(std::move(dialog));
                }
            } else {
                dialogs.push_back(std::move(dialog));
            }
        }
    }

    bool DialogContainer::renderHeader(DialogSettings &settings)
    {
        std::string id = settings.header + "##" + std::to_string(reinterpret_cast<uintptr_t>(&settings));

        if (!ImGui::IsPopupOpen(id.c_str()))
            ImGui::OpenPopup(id.c_str());

        ImGui::SetNextWindowSize({ 500, 400 }, ImGuiCond_FirstUseEver);
        if (ImGui::BeginPopupModal(id.c_str())) {

            ImVec2 size = ImGui::GetContentRegionAvail();
            size.x -= 4.0f;
            size.y -= 24.0f;
            ImGui::BeginChild("Main", size);

            return true;
        } else {
            return false;
        }
    }

    void DialogContainer::renderFooter(DialogSettings &settings)
    {
        ImGui::EndChild();

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

        ImGui::EndPopup();
    }

}
}
