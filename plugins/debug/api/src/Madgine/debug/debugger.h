#pragma once

#include "Generic/execution/stop_callback.h"
#include "Madgine/root/rootcomponentbase.h"
#include "Madgine/root/rootcomponentcollector.h"

#include "continuation.h"
#include "debuglocation.h"

namespace Engine {
namespace Debug {

    struct MADGINE_DEBUGGER_EXPORT ContextInfo : BaseLocation {
        ContextInfo()
            : BaseLocation { this }
        {
        }

        void stepInto(DebugLocation &child) override;
        void stepOut(DebugLocation &child) override;

        void suspend(const DebugLocation &location, Continuation callback, Continuation &outContinuation, Execution::StopToken st);
        void continueExecution(ContinuationMode mode);

        ContinuationMode resume();
        ContinuationMode step();
        std::nullopt_t pause();
        ContinuationMode stop();

        bool alive() const;
        bool isPaused() const;

        mutable std::mutex mMutex;

        friend struct Debugger;

        DebugLocation *mChild = nullptr;

    private:
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

        bool wantsPause(const DebugLocation &location, ContinuationType type, IndexType<size_t> line);

        mutable std::mutex mMutex;

    private:
        std::deque<ContextInfo> mContexts;
        std::vector<DebugListener *> mListeners;
    };

}
}