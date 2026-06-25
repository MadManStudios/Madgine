#include "../toolslib.h"

#include "tasktracker.h"

#include "Modules/threading/taskqueue.h"
#include "Modules/threading/workgroup.h"
#include "Modules/uniquecomponent/uniquecomponentcollector.h"

#include "Meta/reflect/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include "imgui/imguiaddons.h"

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
        if (beginToolPanel("TaskTracker", &mVisible, ImGuiDir_Up)) {

            ImGui::Checkbox("Locked", &mLocked);
            if (!mLocked)
                mEnd = std::chrono::high_resolution_clock::now();

            long long fullPlotSize = std::chrono::duration_cast<std::chrono::nanoseconds>(mEnd - mStart).count();
            ImDrawList *draw_list = ImGui::GetWindowDrawList();

            ImGuiIO &io = ImGui::GetIO();

            bool isHovered = false;
            float mouseRatio;

            mNextHoveredTracker = nullptr;
            mNextHoveredId = nullptr;

            std::chrono::high_resolution_clock::time_point timeAreaBegin = mStart + std::chrono::nanoseconds { mScroll };
            std::chrono::high_resolution_clock::time_point timeAreaEnd = timeAreaBegin + std::chrono::nanoseconds { static_cast<long long>(1000000000 / mZoom) };

            ImGui::Columns(2);

            std::vector<std::pair<const char *, std::reference_wrapper<Debug::Tasks::TaskTracker>>> trackers;
            for (Threading::TaskQueue *queue : Threading::WorkGroup::self().taskQueues()) {
                trackers.emplace_back(queue->name().c_str(), queue->mTracker);
            }
            std::ranges::copy(mCustomTrackers, std::back_inserter(trackers));

            for (auto [name, _tracker] : trackers) {
                Debug::Tasks::TaskTracker &tracker = _tracker;
                ImGui::Text("%s", name);

                ImGui::NextColumn();

                Math::Rect2 threadPlotRect = beginPlot(name);

                isHovered |= ImGui::IsItemHovered(); // Hovered

                ImGui::SetItemKeyOwner(ImGuiKey_MouseWheelY);

                ImGui::NextColumn();

                ImGui::Text("\tThread [%llu]", tracker.mThread);

                ImGui::NextColumn();

                ImGui::PushID(std::hash<std::thread::id> {}(tracker.mThread));
                Math::Rect2 plotRect = beginPlot(name);
                ImGui::PopID();
                mouseRatio = (ImGui::GetMousePos().x - plotRect.mTopLeft.x) / plotRect.mSize.x;
                ImVec2 keep = ImGui::GetCursorScreenPos();

                isHovered |= ImGui::IsItemHovered(); // Hovered

                ImGui::SetItemKeyOwner(ImGuiKey_MouseWheelY);

                std::lock_guard guard { tracker.mMutex };
                auto begin = tracker.events().begin();
                auto end = tracker.events().end();
                auto cmp1 = [](const Debug::Tasks::TaskTracker::Event &event, const std::chrono::high_resolution_clock::time_point &t) { return event.mTimePoint < t; };
                auto start = std::lower_bound(begin, end, timeAreaBegin - 50ms, cmp1);
                auto cmp2 = [](const std::chrono::high_resolution_clock::time_point &t, const Debug::Tasks::TaskTracker::Event &event) { return t < event.mTimePoint; };
                end = std::upper_bound(start, end, timeAreaEnd + 50ms, cmp2);

                auto it = start;

                size_t i = 0;
                float lastResumeX = 0.0f;
                void *id = nullptr;

                float currentX = 0.0f;
                while (it != end) {
                    const Debug::Tasks::TaskTracker::Event &ev = *it;
                    ++it;

                    float x = getEventCoordinate(ev.mTimePoint, plotRect.mSize.x);

                    switch (ev.mType) {
                    case Debug::Tasks::TaskTracker::Event::RESUME: {
                        lastResumeX = x;
                        currentX = x;
                        id = plotRecurse(it, end, currentX, ev.mIdentifier, ev.mDepth, plotRect, i++);
                        // assert(suspended || it == end);
                        break;
                    }
                    case Debug::Tasks::TaskTracker::Event::SUSPEND:
                        plot(lastResumeX, x, ev.mDepth, plotRect, id, (i++) % 2, 0.0f != currentX);
                        if (lastResumeX != currentX) {
                            plot(currentX, x, ev.mDepth + 1, plotRect, id, (i - 1) % 2);
                        }
                        id = nullptr;
                        currentX = x;
                        break;
                    case Debug::Tasks::TaskTracker::Event::RETURN:
                        plot(lastResumeX, x, ev.mDepth, plotRect, id, (i++) % 2, 0.0f != currentX);
                        if (lastResumeX != currentX) {
                            plot(currentX, x, ev.mDepth + 1, plotRect, id, (i - 1) % 2);
                        }
                        id = ev.mIdentifier;
                        currentX = x;
                        break;
                    case Debug::Tasks::TaskTracker::Event::ENTER:
                        if (x != currentX) {
                            plot(currentX, x, ev.mDepth, plotRect, id, (i++) % 2);
                        }
                        currentX = x;
                        id = plotRecurse(it, end, currentX, ev.mIdentifier, ev.mDepth, plotRect, i++);
                        break;
                    }
                }

                ImGui::SetCursorScreenPos(keep);

                ImGui::NextColumn();

                if (!mNextHoveredTracker && mNextHoveredId) {
                    mNextHoveredTracker = &tracker;
                }
            }

            ImGui::Text("");

            ImGui::NextColumn();

            ImVec2 canvas_p0 = ImGui::GetCursorScreenPos(); // ImDrawList API uses screen coordinates!
            ImVec2 scrollbar_sz = { canvas_p0.x + ImGui::GetContentRegionAvail().x, canvas_p0.y + 20.0f }; // Resize canvas to what's available

            ImGui::ScrollbarEx({ canvas_p0, scrollbar_sz }, 1, ImGuiAxis_X, &mScroll, 1000000000 / mZoom, fullPlotSize, ImDrawFlags_RoundCornersAll);

            ImGui::EndColumns();

            // ImGui::Text("%lld < %lld < %lld", 0, mScroll, fullPlotSize);
            // ImGui::DragScalar("debug", ImGuiDataType_S64, &mScroll);

            if (isHovered) {
                float factor = powf(1.1f, io.MouseWheel);
                mZoom *= factor;
                long long mouseOffset = std::chrono::duration_cast<std::chrono::nanoseconds>(timeAreaEnd - timeAreaBegin).count() * mouseRatio;
                mScroll += mouseOffset * (1.0f - 1.0f / factor);
            }

            if (mHoveredTracker != mNextHoveredTracker || mHoveredId != mNextHoveredId) {
                mHoveredTracker = mNextHoveredTracker;
                mHoveredId = mNextHoveredId;
                mHoveredAssignTimepoint = mStart;
            } else if (mHoveredId) {
                ImGui::BeginTooltip();
                ImGui::Text("%s", mHoveredTracker->getTraceback(mHoveredId).function_name());
                if (ImGui::IsKeyDown(ImGuiMod_Ctrl)) {
                    std::chrono::steady_clock::time_point last = std::chrono::steady_clock::time_point::min();
                    std::lock_guard guard { mHoveredTracker->mMutex };
                    for (const auto &event : mHoveredTracker->events()) {
                        if (event.mIdentifier != mHoveredId)
                            continue;
                        std::string type;
                        switch (event.mType) {
                        case Debug::Tasks::TaskTracker::Event::ENTER:
                            type = "ENTER";
                            break;
                        case Debug::Tasks::TaskTracker::Event::RESUME:
                            type = "RESUME";
                            break;
                        case Debug::Tasks::TaskTracker::Event::RETURN:
                            type = "RETURN";
                            break;
                        case Debug::Tasks::TaskTracker::Event::SUSPEND:
                            type = "SUSPEND";
                            break;
                        }
                        float diff = 0.0f;
                        if (last != std::chrono::steady_clock::time_point::min()) {
                            diff = std::chrono::duration_cast<std::chrono::duration<float, std::milli>>(event.mTimePoint - last).count();
                        }
                        last = event.mTimePoint;
                        ImGui::Text("%s: (%hu) %s, %f ms", type.c_str(), event.mDepth, mHoveredTracker->mTasksInFlight.at(event.mIdentifier).function_name(), diff);
                    }
                }
                ImGui::EndTooltip();
            }

            ImGui::Separator();

            for (auto [name, _tracker] : trackers) {
                Debug::Tasks::TaskTracker &tracker = _tracker;
                if (ImGui::TreeNode(name)) {
                    if (ImGui::TreeNode("tasks", "Tasks in Flight")) {
                        std::lock_guard guard { tracker.mMutex };
                        for (auto &[id, stacktrace] : tracker.tasksInFlight()) {
                            ImGui::Text("%s", stacktrace.function_name());
                        }
                        ImGui::TreePop();
                    }
                    if (ImGui::TreeNode("events")) {
                        int i = 0;
                        std::lock_guard guard { tracker.mMutex };
                        for (const auto &event : tracker.events()) {
                            std::string type;
                            switch (event.mType) {
                            case Debug::Tasks::TaskTracker::Event::ENTER:
                                type = "ENTER";
                                break;
                            case Debug::Tasks::TaskTracker::Event::RESUME:
                                type = "RESUME";
                                break;
                            case Debug::Tasks::TaskTracker::Event::RETURN:
                                type = "RETURN";
                                break;
                            case Debug::Tasks::TaskTracker::Event::SUSPEND:
                                type = "SUSPEND";
                                break;
                            }
                            ImGui::Text("%s: (%hu) %s", type.c_str(), event.mDepth, event.mIdentifier ? tracker.mTasksInFlight.at(event.mIdentifier).function_name() : "");
                            if (i++ == 1000)
                                break;
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

    void TaskTracker::registerCustomTracker(const char *name, Debug::Tasks::TaskTracker &tracker)
    {
        mCustomTrackers.emplace_back(name, tracker);
    }

    Math::Rect2 TaskTracker::beginPlot(const char *name)
    {
        // Using InvisibleButton() as a convenience 1) it will advance the layout cursor and 2) allows us to use IsItemHovered()/IsItemActive()
        Math::Vector2 canvas_p0 = ImGui::GetCursorScreenPos(); // ImDrawList API uses screen coordinates!
        Math::Vector2 canvas_sz = { ImGui::GetContentRegionAvail().x, 70.0f }; // Resize canvas to what's available
        if (canvas_sz.x < 50.0f)
            canvas_sz.x = 50.0f;
        Math::Vector2 canvas_p1 = canvas_p0 + canvas_sz;

        // Draw border and background color
        ImDrawList *draw_list = ImGui::GetWindowDrawList();
        draw_list->AddRectFilled(canvas_p0, canvas_p1, IM_COL32(50, 50, 50, 255));
        draw_list->AddRect(canvas_p0, canvas_p1, IM_COL32(255, 255, 255, 255));

        // This will catch our interactions
        ImGui::InvisibleButton(name, canvas_sz, ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight);

        mLastXEnd = -100.0f;

        return { canvas_p0 + Math::Vector2 { 2, 2 }, canvas_sz - Math::Vector2 { 4, 4 } };
    }

    void TaskTracker::endPlot()
    {
    }

    void TaskTracker::plot(float x, float x_end, size_t depth, const Math::Rect2 &plotRect, void *id, size_t colorIndex, bool singleDepth)
    {
        if (x == x_end) {
            if (mLastXEnd == x)
                return;
            --x;
        }
        ImDrawList *draw_list = ImGui::GetWindowDrawList();

        float top = singleDepth ? plotRect.bottom() - 5.0f * (depth + 1) : plotRect.mTopLeft.y;

        ImU32 color = colors[colorIndex];
        if (mHoveredId == id)
            color += IM_COL32(50, 50, 50, 0);
        draw_list->AddRectFilled({ plotRect.mTopLeft.x + x, top }, { plotRect.mTopLeft.x + x_end, plotRect.bottom() - 5.0f * depth }, color);

        if (ImGui::IsMouseHoveringRect({ plotRect.mTopLeft.x + x, top }, { plotRect.mTopLeft.x + x_end, plotRect.bottom() - 5.0f * depth }, false)) {
            mNextHoveredId = id;
            if (mHoveredId == id && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
                int i = 3;
        }
        mLastXEnd = x_end;
    }

    void *TaskTracker::plotRecurse(std::deque<Debug::Tasks::TaskTracker::Event>::const_iterator &it, const std::deque<Debug::Tasks::TaskTracker::Event>::const_iterator &end, float &currentX, void *id, size_t depth, const Math::Rect2 &plotRect, size_t selfI)
    {
        float startX = currentX;
        size_t i = 0;
        size_t color = ((selfI % 2) * 2) + (depth % 2);

        while (it != end) {
            const Debug::Tasks::TaskTracker::Event &ev = *it;
            ++it;

            float x = getEventCoordinate(ev.mTimePoint, plotRect.mSize.x);

            switch (ev.mType) {
            case Debug::Tasks::TaskTracker::Event::RESUME: {
                if (x != currentX) {
                    plot(currentX, x, depth + 1, plotRect, id, color);
                }
                currentX = x;
                void *innerId = plotRecurse(it, end, currentX, ev.mIdentifier, ev.mDepth, plotRect, i++);
                assert(!innerId || it == end);
                break;
            }
            case Debug::Tasks::TaskTracker::Event::SUSPEND:
                plot(startX, x, depth, plotRect, id, color, startX != currentX);
                if (startX != currentX) {
                    plot(currentX, x, depth + 1, plotRect, id, color);
                }
                currentX = x;
                return nullptr;
            case Debug::Tasks::TaskTracker::Event::RETURN:
                plot(startX, x, depth, plotRect, id, color, startX != currentX);
                if (startX != currentX) {
                    plot(currentX, x, depth + 1, plotRect, id, color);
                }
                currentX = x;
                return ev.mIdentifier;
            case Debug::Tasks::TaskTracker::Event::ENTER: {
                if (x != currentX) {
                    plot(currentX, x, depth + 1, plotRect, id, color);
                }
                currentX = x;
                void *innerId = plotRecurse(it, end, currentX, ev.mIdentifier, depth + 1, plotRect, i++);
                if (!innerId) {
                    plot(startX, currentX, depth, plotRect, id, color, true);
                    return nullptr;
                }
                break;
            }
            }
        }

        plot(startX, plotRect.mSize.x, depth, plotRect, id, color, startX != currentX);
        if (startX != currentX) {
            plot(currentX, plotRect.mSize.x, depth + 1, plotRect, id, color);
        }
        currentX = plotRect.mSize.x;
        return id;
    }

    float TaskTracker::getEventCoordinate(std::chrono::high_resolution_clock::time_point t, float pixelWidth)
    {
        long long timePoint = std::chrono::duration_cast<std::chrono::nanoseconds>(t - mStart).count() - mScroll;
        float scaledPoint = (timePoint / 1000000000.0f) * mZoom;
        scaledPoint = Math::clamp(scaledPoint, 0.0f, 1.0f);
        return std::round(scaledPoint * pixelWidth);
    }
}
}
