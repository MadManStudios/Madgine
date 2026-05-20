#include "../debugtoolslib.h"

#include "continuationlist.h"

#include "Madgine/debug/continuation.h"

#include "Madgine_Tools/imguiicons.h"
#include "imgui/imgui.h"
#include "imgui/imguiaddons.h"

namespace Engine {
namespace Tools {

    ContinuationList::ContinuationList(ControlButton button)
        : mButton(button)
    {
    }

    ContinuationList::~ContinuationList()
    {
        for (auto &[continuation, mode] : mContinuations) {
            continuation(mode);
        }
    }

    void ContinuationList::controls(Debug::Continuation &continuation)
    {
        ImGui::PushID(&continuation);

        ControlButton button = mButton;

        if (ImGui::InlineContextButton(IMGUI_ICON_STOP)) {
            button = ControlButton::STOP;
        }

        if (ImGui::InlineContextButton(IMGUI_ICON_STEP)) {
            button = ControlButton::STEP;
        }

        if (ImGui::InlineContextButton(IMGUI_ICON_PLAY)) {
            button = ControlButton::PLAY;
        }

        ImGui::PopID();
        switch (button) {
        case ControlButton::PLAY:
            mContinuations.emplace_back(std::move(continuation), Debug::ContinuationMode::Continue);
            break;
        case ControlButton::STEP:
            mContinuations.emplace_back(std::move(continuation), Debug::ContinuationMode::Continue);
            break;
        case ControlButton::STOP:
            mContinuations.emplace_back(std::move(continuation), Debug::ContinuationMode::Abort);
            break;
        case ControlButton::PAUSE:
            throw 0;
            break;
        case ControlButton::NONE:
            break;
        }
    }

}
}