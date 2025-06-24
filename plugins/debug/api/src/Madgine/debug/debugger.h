#pragma once

#include "Generic/callable_view.h"
#include "Generic/closure.h"
#include "Generic/execution/statedescriptor.h"
#include "Generic/execution/stop_callback.h"
#include "Generic/execution/virtualsender.h"
#include "Generic/forward_capture.h"
#include "Madgine/root/rootcomponentbase.h"
#include "Madgine/root/rootcomponentcollector.h"

#include "continuation.h"
#include "debuglocation.h"

namespace Engine {
namespace Debug {

    struct MADGINE_DEBUGGER_EXPORT ContextInfo : ParentLocation, Execution::StopCallback {
        ContextInfo()
            : ParentLocation { nullptr, this }
        {
            mRunning.test_and_set();
        }

        void suspend(Continuation callback, Execution::StopToken st);
        void continueExecution(ContinuationMode mode);

        ContinuationMode resume();
        ContinuationMode step();
        std::nullopt_t pause();
        ContinuationMode stop();

        bool alive() const;
        bool isPaused() const;

        std::string getArguments() const;
        ContinuationType continuationType() const;

        mutable std::mutex mMutex;

        friend struct Debugger;

    protected:
        void stopRequested() override;

    private:
        Continuation mCallback;
        std::atomic_flag mRunning;
        bool mPauseRequested = false;
        bool mStopRequested = false;
    };

    struct MADGINE_DEBUGGER_EXPORT Debugger : Root::RootComponent<Debugger> {

        friend struct ContextInfo;

        Debugger(const Debugger &) = delete;

        using Root::RootComponent<Debugger>::RootComponent;

        virtual std::string_view key() const override;

        std::deque<ContextInfo> &infos();
        ContextInfo &createContext();

        void addListener(DebugListener *listener);
        void removeListener(DebugListener *listener);

        bool wantsPause(const DebugLocation &location, ContinuationType type);

        mutable std::mutex mMutex;

    private:
        std::deque<ContextInfo> mContexts;
        std::vector<DebugListener *> mListeners;
    };

}
}