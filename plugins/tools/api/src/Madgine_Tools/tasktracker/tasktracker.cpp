#include "../toolslib.h"

#include "tasktracker.h"

#include "imgui/imgui.h"
#include "imgui/imguiaddons.h"

#include "Meta/serialize/serializetable_impl.h"

#include "Meta/keyvalue/metatable_impl.h"
#include "Modules/uniquecomponent/uniquecomponentcollector.h"

#include "Modules/threading/taskqueue.h"
#include "Modules/threading/workgroup.h"

#include "imgui/imgui_internal.h"

UNIQUECOMPONENT(Engine::Tools::TaskTracker);

METATABLE_BEGIN_BASE(Engine::Tools::TaskTracker, Engine::Tools::ToolBase)
METATABLE_END(Engine::Tools::TaskTracker)

SERIALIZETABLE_INHERIT_BEGIN(Engine::Tools::TaskTracker, Engine::Tools::ToolBase)
SERIALIZETABLE_END(Engine::Tools::TaskTracker)

namespace Engine {
namespace Tools {

    static constexpr ImU32 colors[4] = {
        IM_COL32(200, 50, 50, 255),
        IM_COL32(150, 150, 50, 255),
        IM_COL32(50, 50, 200, 255),
        IM_COL32(50, 150, 150, 255)
    };

    TaskTracker::TaskTracker(ImRoot &root)
        : Tool<TaskTracker>(root)
    {
    }

    TaskTracker::~TaskTracker()
    {
    }

    void TaskTracker::render()
    {
        if (ImGui::Begin("TaskTracker", &mVisible)) {

            ImGui::Checkbox("Locked", &mLocked);
            if (!mLocked)
                mEnd = std::chrono::high_resolution_clock::now();

            long long fullPlotSize = std::chrono::duration_cast<std::chrono::nanoseconds>(mEnd - mStart).count();
            ImDrawList *draw_list = ImGui::GetWindowDrawList();

            ImGuiIO &io = ImGui::GetIO();

            bool isHovered = false;
            float mouseRatio;

            Debug::Tasks::TaskTracker *hoveredTracker = nullptr;
            void *hoveredId = nullptr;

            std::chrono::high_resolution_clock::time_point timeAreaBegin = mStart + std::chrono::nanoseconds { mScroll };
            std::chrono::high_resolution_clock::time_point timeAreaEnd = timeAreaBegin + std::chrono::nanoseconds { static_cast<long long>(1000000000 / mZoom) };

            ImGui::Columns(2);

            std::vector<std::pair<const char *, Debug::Tasks::TaskTracker *>> trackers; 
            for (Threading::TaskQueue *queue : Threading::WorkGroup::self().taskQueues()) {
                trackers.emplace_back(queue->name().c_str(), &queue->mTracker);
            }
            std::ranges::copy(mCustomTrackers, std::back_inserter(trackers));

            for (auto [name, tracker] : trackers) {
                ImGui::Text("%s", name);

                ImGui::NextColumn();

                Rect2 threadPlotRect = beginPlot();

                isHovered |= ImGui::IsItemHovered(); // Hovered

                ImGui::NextColumn();

                ImGui::Text("\tThread [%llu]", tracker->mThread);

                ImGui::NextColumn();

                Rect2 plotRect = beginPlot();
                mouseRatio = (ImGui::GetMousePos().x - plotRect.mTopLeft.x) / plotRect.mSize.x;
                ImVec2 keep = ImGui::GetCursorScreenPos();

                isHovered |= ImGui::IsItemHovered(); // Hovered

                std::lock_guard guard { tracker->mMutex };
                auto begin = tracker->events().begin();
                auto end = tracker->events().end();
                auto cmp1 = [](const Debug::Tasks::TaskTracker::Event &event, const std::chrono::high_resolution_clock::time_point &t) { return event.mTimePoint < t; };
                auto start = std::lower_bound(begin, end, timeAreaBegin - 50ms, cmp1);
                auto cmp2 = [](const std::chrono::high_resolution_clock::time_point &t, const Debug::Tasks::TaskTracker::Event &event) { return t < event.mTimePoint; };
                end = std::upper_bound(start, end, timeAreaEnd + 50ms, cmp2);

                auto it = start;

                struct Plot {
                    float x;
                    float x_end;
                    void *id;
                    size_t depth;
                };
                int i = 0;
                auto plot = [&, tracker { tracker }](Plot &p) {
                    p.x_end = std::max(p.x_end, p.x + 1.0f);

                    ImU32 color = colors[(i++ + p.depth) % 4];
                    if (mHoveredTracker == tracker && mHoveredId == p.id)
                        color += IM_COL32(50, 50, 50, 0);
                    draw_list->AddRectFilled({ plotRect.mTopLeft.x + p.x, plotRect.mTopLeft.y }, { plotRect.mTopLeft.x + p.x_end, plotRect.bottom() - 5.0f * p.depth }, color);
                    ImGui::SetCursorScreenPos({ plotRect.mTopLeft.x + p.x, plotRect.mTopLeft.y });
                    ImGui::InvisibleButton("id", { p.x_end - p.x, plotRect.mSize.y - 5.0f * p.depth }, ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight);

                    if (ImGui::IsItemHovered()) {
                        hoveredTracker = tracker;
                        hoveredId = p.id;
                    }
                };

                Plot p {
                    0.0f,
                    plotRect.mSize.x,
                    nullptr,
                    0
                };
                size_t callDepth = 0;
                std::stack<Plot> plots;
                std::stack<std::pair<float, void *>> xs;

                for (; it != end; ++it) {
                    const Debug::Tasks::TaskTracker::Event &ev = *it;
                    switch (ev.mType) {
                    case Debug::Tasks::TaskTracker::Event::RESUME:
                        if (callDepth == 0) {
                            p.x = getEventCoordinate(ev.mTimePoint, plotRect.mSize.x);
                            p.id = ev.mIdentifier;
                        } else {
                            xs.push({ getEventCoordinate(ev.mTimePoint, plotRect.mSize.x), ev.mIdentifier });
                        }
                        ++callDepth;
                        break;
                    case Debug::Tasks::TaskTracker::Event::SUSPEND:
                        if (callDepth == 0) {
                            assert(p.x == 0.0f && p.x_end == plotRect.mSize.x);
                            plots.push({ 0.0f, getEventCoordinate(ev.mTimePoint, plotRect.mSize.x), ev.mIdentifier, xs.size() });
                            break;
                        } else if (callDepth == 1) {
                            p.x_end = getEventCoordinate(ev.mTimePoint, plotRect.mSize.x);
                            while (!xs.empty()) {
                                plots.push({ xs.top().first, p.x_end, xs.top().second, xs.size() });
                                xs.pop();
                            }
                            plots.push(p);
                            p.x = 0.0f;
                            p.x_end = plotRect.mSize.x;
                            p.id = nullptr;
                        } else {
                            float x = 0.0f;
                            void *identifier = nullptr;
                            if (!xs.empty()) {
                                //assert(xs.top().second == ev.mIdentifier);
                                identifier = xs.top().second;
                                x = xs.top().first;
                                xs.pop();
                            }
                            plots.push({ x, getEventCoordinate(ev.mTimePoint, plotRect.mSize.x), identifier, xs.size() + 1 });
                        }
                        --callDepth;
                        break;
                    case Debug::Tasks::TaskTracker::Event::ENTER:
                        xs.push({ getEventCoordinate(ev.mTimePoint, plotRect.mSize.x), ev.mIdentifier });
                        break;
                    case Debug::Tasks::TaskTracker::Event::RETURN: {
                        float x = 0.0f;
                        if (!xs.empty()) {
                            assert(xs.top().second == ev.mIdentifier);
                            x = xs.top().first;
                            xs.pop();
                        }
                        plots.push({ x, getEventCoordinate(ev.mTimePoint, plotRect.mSize.x), ev.mIdentifier, xs.size() + 1 });
                    } break;
                    }
                }
                //assert(callDepth > 0 || xs.empty());

                while (!xs.empty()) {
                    plots.push({ xs.top().first, plotRect.mSize.x, xs.top().second, xs.size() });
                    xs.pop();
                }

                if (callDepth > 0)
                    plot(p);
                while (!plots.empty()) {
                    plot(plots.top());
                    plots.pop();
                }

                ImGui::SetCursorScreenPos(keep);

                ImGui::NextColumn();
            }

            ImGui::Text("");

            ImGui::NextColumn();

            ImVec2 canvas_p0 = ImGui::GetCursorScreenPos(); // ImDrawList API uses screen coordinates!
            ImVec2 scrollbar_sz = { canvas_p0.x + ImGui::GetContentRegionAvail().x, canvas_p0.y + 20.0f }; // Resize canvas to what's available

            ImGui::ScrollbarEx({ canvas_p0, scrollbar_sz }, 1, ImGuiAxis_X, &mScroll, 1000000000 / mZoom, fullPlotSize, ImDrawFlags_RoundCornersAll);

            ImGui::EndColumns();

            //ImGui::Text("%lld < %lld < %lld", 0, mScroll, fullPlotSize);
            //ImGui::DragScalar("debug", ImGuiDataType_S64, &mScroll);

            if (isHovered) {
                float factor = powf(1.1f, io.MouseWheel);
                mZoom *= factor;
                long long mouseOffset = std::chrono::duration_cast<std::chrono::nanoseconds>(timeAreaEnd - timeAreaBegin).count() * mouseRatio;
                mScroll += mouseOffset * (1.0f - 1.0f / factor);
            }

            if (mHoveredTracker != hoveredTracker || mHoveredId != hoveredId) {
                mHoveredTracker = hoveredTracker;
                mHoveredId = hoveredId;
                mHoveredAssignTimepoint = mStart;
            } else if (mHoveredId) {
                ImGui::BeginTooltip();
                ImGui::Text("%s", mHoveredTracker->getTraceback(mHoveredId).mFunction);
                ImGui::EndTooltip();
            }

            ImGui::Separator();

            for (Threading::TaskQueue *queue : Threading::WorkGroup::self().taskQueues()) {
                if (ImGui::TreeNode(queue->name().c_str())) {
                    if (ImGui::TreeNode("tasks", "Tasks in Flight (%zu)", queue->taskInFlightCount())) {
                        std::lock_guard guard { queue->mTracker.mMutex };
                        for (auto &[id, stacktrace] : queue->mTracker.tasksInFlight()) {
                            ImGui::Text("%s", stacktrace.calculateReadable().front().mFunction);
                        }
                        ImGui::TreePop();
                    }
                    ImGui::TreePop();
                }
            }
        }
        ImGui::End();
    }

    std::string_view TaskTracker::key() const
    {
        return "TaskTracker";
    }

    void TaskTracker::registerCustomTracker(const char *name, Debug::Tasks::TaskTracker *tracker)
    {
        mCustomTrackers.emplace_back(name, tracker);
    }

    Rect2 TaskTracker::beginPlot()
    {
        // Using InvisibleButton() as a convenience 1) it will advance the layout cursor and 2) allows us to use IsItemHovered()/IsItemActive()
        Vector2 canvas_p0 = ImGui::GetCursorScreenPos(); // ImDrawList API uses screen coordinates!
        Vector2 canvas_sz = { ImGui::GetContentRegionAvail().x, 50.0f }; // Resize canvas to what's available
        if (canvas_sz.x < 50.0f)
            canvas_sz.x = 50.0f;
        Vector2 canvas_p1 = canvas_p0 + canvas_sz;

        // Draw border and background color
        ImDrawList *draw_list = ImGui::GetWindowDrawList();
        draw_list->AddRectFilled(canvas_p0, canvas_p1, IM_COL32(50, 50, 50, 255));
        draw_list->AddRect(canvas_p0, canvas_p1, IM_COL32(255, 255, 255, 255));

        // This will catch our interactions
        ImGui::InvisibleButton("canvas", canvas_sz, ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight);

        return { canvas_p0 + Vector2 { 2, 2 }, canvas_sz - Vector2 { 4, 4 } };
    }

    void TaskTracker::endPlot()
    {
    }

    float TaskTracker::getEventCoordinate(std::chrono::high_resolution_clock::time_point t, float pixelWidth)
    {
        long long timePoint = std::chrono::duration_cast<std::chrono::nanoseconds>(t - mStart).count() - mScroll;
        float scaledPoint = (timePoint / 1000000000.0f) * mZoom;
        if (scaledPoint < 0)
            scaledPoint = 0.0f;
        if (scaledPoint > 1)
            scaledPoint = 1.0f;
        return scaledPoint * pixelWidth;
    }
}
}
