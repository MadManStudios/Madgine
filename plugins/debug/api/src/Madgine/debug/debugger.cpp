#include "../debuglib.h"

#include "debugger.h"

#include "Modules/uniquecomponent/uniquecomponentcollector.h"

#include "Meta/keyvalue/metatable_impl.h"

#include "debuglistener.h"

#include "Generic/execution/stop_source.h"

UNIQUECOMPONENT(Engine::Debug::Debugger)

METATABLE_BEGIN(Engine::Debug::Debugger)
METATABLE_END(Engine::Debug::Debugger)

METATABLE_BEGIN(Engine::Debug::ContextInfo)
READONLY_PROPERTY(Alive, alive)
METATABLE_END(Engine::Debug::ContextInfo)

namespace Engine {
namespace Debug {

    std::string_view Debugger::key() const
    {
        return "Debugger";
    }

    std::deque<ContextInfo> &Debugger::infos()
    {
        return mContexts;
    }

    ContextInfo &Debugger::createContext()
    {
        std::unique_lock lock { mMutex };
        return mContexts.emplace_back();
    }

    void Debugger::addListener(DebugListener *listener)
    {
        mListeners.push_back(listener);
    }

    void Debugger::removeListener(DebugListener *listener)
    {
        std::erase(mListeners, listener);
    }

    void ContextInfo::suspend(Continuation callback, Continuation &outContinuation, Execution::StopToken st)
    {
        outContinuation = std::move(callback); //TODO proper syncing with stop_source; see debuggablesender::stop()

        if (mStopRequested || st->stop_requested()) {
            outContinuation(ContinuationMode::Abort);
            return;
        }

        for (DebugListener *listener : Debugger::getSingleton().mListeners)
            listener->onSuspend(*this, outContinuation.type());        
    }

    void ContextInfo::continueExecution(ContinuationMode mode)
    {
        throw 0;
    }

    ContinuationMode ContextInfo::resume()
    {
        mPauseRequested = false;
        return ContinuationMode::Continue;
    }

    ContinuationMode ContextInfo::step()
    {
        mPauseRequested = true;
        return ContinuationMode::Continue;
    }

    std::nullopt_t ContextInfo::pause()
    {
        mPauseRequested = true;
        return std::nullopt;
    }

    ContinuationMode ContextInfo::stop()
    {
        mStopRequested = true;
        return ContinuationMode::Abort;
    }

    bool ContextInfo::alive() const
    {
        return mChild;
    }

    bool ContextInfo::isPaused() const
    {
        return false;
    }
        
    bool Debugger::wantsPause(const DebugLocation &location, ContinuationType type, IndexType<size_t> line)
    {
        bool pause = (line && location.mContext->mPauseRequested) || location.mContext->mStopRequested;

        for (DebugListener *listener : mListeners) {
            pause |= listener->wantsPause(location, type, line);
        }

        return pause;
    }

}
}
