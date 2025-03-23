#pragma once

#include "../toolbase.h"
#include "../toolscollector.h"

#include "Meta/math/rect2.h"

namespace Engine {
namespace Tools {

    struct MADGINE_TOOLS_EXPORT TaskTracker : Tool<TaskTracker> {
        SERIALIZABLEUNIT(TaskTracker)

        TaskTracker(ImRoot &root);
        ~TaskTracker();

        virtual void render() override;

        std::string_view key() const override;

        void registerCustomTracker(const char *name, Debug::Tasks::TaskTracker *tracker);

    protected:
        Rect2 beginPlot();
        void endPlot();

        float getEventCoordinate(std::chrono::high_resolution_clock::time_point t, float pixelWidth);

    private:
        std::chrono::high_resolution_clock::time_point mStart = std::chrono::high_resolution_clock::now() - std::chrono::milliseconds { 10 }, mEnd = mStart;
        long long mScroll = 0;
        bool mLocked = false;
        float mZoom = 10.0f;

        std::vector<std::pair<const char *, Debug::Tasks::TaskTracker *>> mCustomTrackers;

        std::chrono::high_resolution_clock::time_point mHoveredAssignTimepoint;
        Debug::Tasks::TaskTracker *mHoveredTracker = nullptr;
        void *mHoveredId = nullptr;
    };

}
}

REGISTER_TYPE(Engine::Tools::TaskTracker)