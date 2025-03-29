#include "../toolslib.h"

#include "profiler.h"

#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include "imgui/imguiaddons.h"

#include "Modules/debug/profiler/profiler.h"
#include "Modules/debug/profiler/profilerthread.h"
#include "Modules/debug/profiler/processstats.h"

#include "Meta/keyvalue/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

#include "Modules/uniquecomponent/uniquecomponentcollector.h"

UNIQUECOMPONENT(Engine::Tools::Profiler);

namespace Engine {
namespace Tools {

    static void drawStats(const Debug::Profiler::ProcessStats *stats, std::chrono::nanoseconds overallTime, std::chrono::nanoseconds parentTotalTime)
    {
        std::chrono::nanoseconds totalTime = stats->totalTime();

        ImGui::TableNextRow();

        ImGui::TableNextColumn();

        ImGui::BeginSpanningTreeNode(stats, stats->function(), stats->children().empty() ? ImGuiTreeNodeFlags_Leaf : ImGuiTreeNodeFlags_None);

        ImGui::TableNextColumn();

        ImGui::RightAlignDuration(totalTime);

        ImGui::TableNextColumn();

        ImGui::RightAlignText("%.2f%%", 100.0f * totalTime.count() / parentTotalTime.count());

        ImGui::TableNextColumn();

        ImGui::RightAlignText("%.2f%%", 100.0f * totalTime.count() / overallTime.count());

        if (ImGui::EndSpanningTreeNode()) {
            for (const std::pair<Debug::Profiler::ProcessStats *const, Debug::Profiler::ProcessStats::Data> &child : stats->children()) {
                drawStats(child.first, overallTime, totalTime);
            }
            ImGui::TreePop();
        }
    }

    Profiler::Profiler(ImRoot &root)
        : Tool<Profiler>(root)
        , mProfiler(Debug::Profiler::Profiler::getCurrent())
    {
    }

    void Profiler::render()
    {
        if (ImGui::Begin("Profiler", &mVisible)) {
            
             if (ImGui::BeginTable("cols", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY | ImGuiTableFlags_Hideable | ImGuiTableFlags_Resizable)) {

                ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_NoHide);
                ImGui::TableSetupColumn("Total time");
                ImGui::TableSetupColumn("Rel. Time (parent)");
                ImGui::TableSetupColumn("Rel. Time (total)");         

                ImGui::TableSetupScrollFreeze(0, 1);

                ImGui::TableHeadersRow();

                for (const Debug::Profiler::ProfilerThread *thread : mProfiler.getThreadStats()) {
                    drawStats(&thread->mStats, thread->mStats.totalTime(), thread->mStats.totalTime());
                }

                ImGui::EndTable();
            }
        }
        ImGui::End();
    }

    std::string_view Profiler::key() const
    {
        return "Profiler";
    }

}
}

METATABLE_BEGIN_BASE(Engine::Tools::Profiler, Engine::Tools::ToolBase)
METATABLE_END(Engine::Tools::Profiler)

SERIALIZETABLE_INHERIT_BEGIN(Engine::Tools::Profiler, Engine::Tools::ToolBase)
SERIALIZETABLE_END(Engine::Tools::Profiler)

