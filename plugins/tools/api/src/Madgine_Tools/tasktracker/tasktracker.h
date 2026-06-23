#pragma once

#include "Meta/math/rect2.h"

#include "../toolbase.h"
#include "../toolscollector.h"

namespace Engine {
namespace Tools {

    struct MADGINE_TOOLS_EXPORT TaskTracker : Tool<TaskTracker> {
        SERIALIZABLEUNIT(TaskTracker)

        TaskTracker(ImRoot &root);
        ~TaskTracker();

        virtual void render() override;

        std::string_view key() const override;

        void registerCustomTracker(const char *name, Debug::Tasks::TaskTracker &tracker);

    protected:
        Math::Rect2 beginPlot(const char *name);
        void endPlot();

        void plot(float x, float x_end, size_t depth, const Math::Rect2 &plotRect, void *id, size_t colorIndex, bool singleDepth = false);

        void *plotRecurse(std::deque<Debug::Tasks::TaskTracker::Event>::const_iterator &it, const std::deque<Debug::Tasks::TaskTracker::Event>::const_iterator &end, float &startX, void *id, size_t depth, const Math::Rect2 &plotRect, size_t selfI);

        float getEventCoordinate(std::chrono::high_resolution_clock::time_point t, float pixelWidth);

    private:
        std::chrono::high_resolution_clock::time_point mStart = std::chrono::high_resolution_clock::now() - std::chrono::milliseconds { 10 }, mEnd = mStart;
        long long mScroll = 0;
        bool mLocked = false;
        float mZoom = 10.0f;

        float mLastXEnd;

        std::vector<std::pair<const char *, std::reference_wrapper<Debug::Tasks::TaskTracker>>> mCustomTrackers;

        std::chrono::high_resolution_clock::time_point mHoveredAssignTimepoint;
        Debug::Tasks::TaskTracker *mHoveredTracker = nullptr;
        void *mHoveredId = nullptr;
        Debug::Tasks::TaskTracker *mNextHoveredTracker = nullptr;
        void *mNextHoveredId = nullptr;
    };

}
}