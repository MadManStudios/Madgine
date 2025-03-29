#include "../toolslib.h"

#include "metrics.h"

#include "imgui/imgui.h"
#include "imgui/imguiaddons.h"

#include "Meta/serialize/serializetable_impl.h"

#include "Meta/keyvalue/metatable_impl.h"
#include "Modules/uniquecomponent/uniquecomponentcollector.h"

#include "../renderer/imroot.h"

#include "Generic/projections.h"

UNIQUECOMPONENT(Engine::Tools::Metrics);

namespace Engine {
namespace Tools {

    Metrics::Metrics(ImRoot &root)
        : Tool<Metrics>(root)
    {
    }

    void Metrics::render()
    {
        ImGui::SetNextWindowSize({ 500, 125 }, ImGuiCond_FirstUseEver);

        if (ImGui::Begin("Metrics", &mVisible)) {
            if (ImGui::CollapsingHeader("FPS")) {
                ImGui::Text("Time/frame: ");
                ImGui::Duration(std::chrono::microseconds(static_cast<long long>(ImGui::GetIO().DeltaTime * 1000000.0f)));
                ImGui::PlotHistory(mFramesTrend, "Frames per Second");
            }

            for (ToolBase *tool : mRoot.tools() | std::views::transform(projectionUniquePtrToPtr)) {
                if (tool->isEnabled())
                    tool->renderMetrics();
            }
        }
        ImGui::End();
    }

    void Metrics::renderStatus()
    {
        ImGui::Text("%.2f FPS", mFramesPerSecond.average());
        ImGui::Separator();
    }

    void Metrics::update()
    {
        if (ImGui::GetIO().DeltaTime > 0.0f) {
            float fps = 1.0f / ImGui::GetIO().DeltaTime;
            mFramesPerSecond << fps;
        }

        mTimeBank += ImGui::GetIO().DeltaTime;
        if (mTimeBank >= 0.5f) {
            mTimeBank = fmodf(mTimeBank, 0.5f);
            mFramesTrend << mFramesPerSecond.average();
        }

        ToolBase::update();
    }

    std::string_view Metrics::key() const
    {
        return "Metrics";
    }

}
}

METATABLE_BEGIN_BASE(Engine::Tools::Metrics, Engine::Tools::ToolBase)
METATABLE_END(Engine::Tools::Metrics)

SERIALIZETABLE_INHERIT_BEGIN(Engine::Tools::Metrics, Engine::Tools::ToolBase)
SERIALIZETABLE_END(Engine::Tools::Metrics)
