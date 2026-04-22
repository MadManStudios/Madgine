#include "../toolslib.h"

#include "dialogs.h"

#include "imgui/imgui.h"

namespace Engine {
namespace Tools {

    void DialogSettings::accept()
    {
        mResult = DialogResult::Accepted;
    }

    void DialogSettings::acceptAll()
    {
        root().mResult = DialogResult::Accepted;
    }

    void DialogSettings::decline()
    {
        mResult = DialogResult::Declined;
    }

    void DialogSettings::declineAll()
    {
        root().mResult = DialogResult::Declined;
    }

    void DialogSettings::cancel()
    {
        mResult = DialogResult::Canceled;
    }

    bool DialogSettings::completed() const
    {
        return (mResult && *mResult != DialogResult::Canceled) || (root().mResult && *root().mResult != DialogResult::Canceled);
    }

    bool DialogSettings::accepted() const
    {
        return (mResult && *mResult == DialogResult::Accepted) || (root().mResult && *root().mResult == DialogResult::Accepted);
    }

    bool DialogSettings::declined() const
    {
        return (mResult && *mResult == DialogResult::Declined) || (root().mResult && *root().mResult == DialogResult::Declined);
    }

    bool DialogSettings::cancelled() const
    {
        return (mResult && *mResult == DialogResult::Canceled) || (root().mResult && *root().mResult == DialogResult::Canceled);
    }

    void DialogSettings::open(CoroutineHandle<DialogPromise> handle)
    {
        mSubDialogs.push_back(std::move(handle));
    }

    void DialogContainer::show(CoroutineHandle<DialogPromise> dialog)
    {
        mDialogs.push_back(std::move(dialog));
    }

    void DialogContainer::showGrouped(std::string_view name, CoroutineHandle<DialogPromise> dialog)
    {
        mDialogGroups.try_emplace(std::string { name }, *this).first->second.addDialog(std::move(dialog));
    }

    DialogContainer::DialogGroup::DialogGroup(DialogContainer &container)
        : mContainer(container)
    {
        mSettings.showCancel = true;
        mSettings.allowApplyToAll = true;
    }

    void DialogContainer::DialogGroup::addDialog(CoroutineHandle<DialogPromise> dialog)
    {
        if (mDialogs.empty()) {
            mSettings.mResult.reset();
        }
        dialog->mSettings.setParent(mSettings);
        mDialogs.emplace_back().push_back(std::move(dialog));
    }

    void DialogContainer::DialogGroup::render()
    {
        if (!mDialogs.empty()) {
            mContainer.handleDialogs(mDialogs.front());
            if (mDialogs.front().empty()) {
                mDialogs.pop_front();
            }            
        }
    }

    void DialogSettings::setParent(DialogSettings &other)
    {
        assert(!mParent);
        mParent = &other;
        allowApplyToAll = other.allowApplyToAll;
        showCancel = other.showCancel;
    }

    void DialogContainer::render()
    {
        handleDialogs(mDialogs);

        for (auto& [name, group] : mDialogGroups) {
            group.render();
        }
    }

    void DialogContainer::handleDialogs(std::vector<CoroutineHandle<DialogPromise>> &dialogs)
    {
        std::vector<CoroutineHandle<DialogPromise>> localDialogs = std::move(dialogs);

        for (CoroutineHandle<DialogPromise> &dialog : localDialogs) {
            if (renderHeader(dialog->mSettings)) {
                assert(!dialog->mContainer);
                dialog->mContainer = &dialog;
                dialog.release().resume();
                assert(dialog);
                dialog->mContainer = nullptr;
                handleDialogs(dialog->mSettings.mSubDialogs);
                renderFooter(dialog->mSettings);
                while (dialog->mSettings.completed()) {
                    if (!dialog.done()) {
                        dialog->mContainer = &dialog;
                        dialog.release().resume();
                        assert(dialog);
                        dialog->mContainer = nullptr;
                    } else {
                        break;
                    }
                }
                if (!dialog.done() && !dialog->mSettings.cancelled()) {
                    dialogs.push_back(std::move(dialog));
                }
            } else {
                dialogs.push_back(std::move(dialog));
            }
        }
    }

    bool DialogContainer::renderHeader(DialogSettings &settings)
    {
        std::string id = settings.header + "##" + std::to_string(reinterpret_cast<uintptr_t>(&settings.root()));

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
            if (settings.allowApplyToAll) {
                if (ImGui::Button((settings.acceptText + " to All").c_str())) {
                    settings.acceptAll();
                }
            }
            if (!settings.acceptPossible)
                ImGui::EndDisabled();
        }

        if (settings.showDecline) {
            if (ImGui::Button(settings.declineText.c_str())) {
                settings.decline();
            }
            if (settings.allowApplyToAll) {
                if (ImGui::Button((settings.declineText + " to All").c_str())) {
                    settings.declineAll();
                }
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

    DialogSettings &DialogSettings::root()
    {
        return mParent ? mParent->root() : *this;
    }

    const DialogSettings &DialogSettings::root() const
    {
        return mParent ? mParent->root() : *this;
    }

}
}
