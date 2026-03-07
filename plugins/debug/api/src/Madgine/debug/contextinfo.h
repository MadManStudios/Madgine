#pragma once

#include "Generic/execution/concepts.h"
#include "Generic/typed_ptr.h"

#include "continuation.h"

namespace Engine {
namespace Debug {

    struct DebugLocation {
        std::vector<bool> mBreakpoints;
    };

    struct MADGINE_DEBUGGER_EXPORT ContextInfo final {

        bool wantsPause(TypedPtr location, ContinuationType type, IndexType<size_t> line);
        void suspend(TypedPtr location, Continuation callback, Continuation &outContinuation, Execution::StopToken st);

        ContinuationMode resume();
        ContinuationMode step();
        std::nullopt_t pause();
        ContinuationMode stop();

        bool alive() const;
        bool isPaused() const;

        mutable std::mutex mMutex;

        void setBreakpoint(const void *location, size_t index, bool set) const;
        bool getBreakpoint(const void *location, size_t index) const;

        template <typename Rec, typename F, typename... Args>
        void yield(TypedPtr location, Rec &rec, F &&callback, Continuation &outContinuation, ContinuationType type, Args &&...args)
        {
            suspend(location, { rec, std::forward<F>(callback), type, std::forward<Args>(args)... }, outContinuation, Execution::get_stop_token(rec));
        }

        template <typename Rec, typename F, typename... Args>
        void pass(TypedPtr location, Rec &rec, F &&callback, Continuation &outContinuation, ContinuationType type, IndexType<size_t> line = {}, Args &&...args)
        {
            if (wantsPause(location, type, line)) {
                yield(location, rec, std::forward<F>(callback), outContinuation, type, std::forward<Args>(args)...);
            } else {
                std::forward<F>(callback)(rec, std::forward<Args>(args)...);
            }
        }

        Debug::SenderLocation *mChild = nullptr;

    private:
        bool mPauseRequested = false;
        bool mStopRequested = false;

        mutable std::map<const void *, DebugLocation> mDebugLocations;
    };

}
}