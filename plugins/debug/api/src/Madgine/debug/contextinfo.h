#pragma once

#include "Generic/execution/concepts.h"
#include "Generic/typed_ptr.h"

namespace Engine {
namespace Debug {

    struct DebugLocation {
        std::vector<bool> mBreakpoints;
    };

    struct MADGINE_DEBUGGER_EXPORT ContextInfo final {

        bool wantsPause(TypedPtr location, ContinuationType type, IndexType<size_t> line);
        void suspend(TypedPtr location, ContinuationType type);

        ContinuationMode resume();
        ContinuationMode step();
        std::nullopt_t pause();
        ContinuationMode stop();

        bool alive() const;
        bool isPaused() const;

        mutable std::mutex mMutex;

        void setBreakpoint(const void *location, size_t index, bool set) const;
        bool getBreakpoint(const void *location, size_t index) const;

        Debug::SenderLocation *mChild = nullptr;

    private:
        bool mStopRequested = false;

        mutable std::map<const void *, DebugLocation> mDebugLocations;
    };

}
}