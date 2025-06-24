#include "../debuglib.h"

#include "debugger.h"

#include "Modules/uniquecomponent/uniquecomponentcollector.h"

#include "Meta/keyvalue/metatable_impl.h"

#include "debuglistener.h"

#include "Generic/execution/stop_source.h"

UNIQUECOMPONENT(Engine::Debug::Debugger)

METATABLE_BEGIN(Engine::Debug::Debugger)
METATABLE_END(Engine::Debug::Debugger)

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

    void ContextInfo::suspend(Continuation callback, Execution::StopToken st)
    {
        if (mStopRequested) {
            callback(ContinuationMode::Abort);
            return;
        }

        for (DebugListener *listener : Debugger::getSingleton().mListeners)
            listener->onSuspend(*this, callback.type());
        mCallback = std::move(callback);        
        mRunning.clear();
        if (!st->registerCallback(this)) {
            if (mRunning.test_and_set())
                callback(ContinuationMode::Abort);
        } 
    }

    void ContextInfo::continueExecution(ContinuationMode mode)
    {
        if (!mRunning.test_and_set()) {
            Closure<void(ContinuationMode)> callback = std::move(mContext->mCallback);
            callback(mode);
        }
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
        return !mRunning.test();
    }

    std::string ContextInfo::getArguments() const
    {
        std::stringstream ss;
        mCallback.visitArguments(ss);
        return ss.str();
    }

    ContinuationType ContextInfo::continuationType() const
    {
        return mCallback.type();
    }

    bool Debugger::wantsPause(const DebugLocation &location, ContinuationType type)
    {
        bool pause = location.mContext->mPauseRequested || location.mContext->mStopRequested;

        for (DebugListener *listener : mListeners) {
            pause |= listener->wantsPause(location, type);
        }

        return pause;
    }

    void ContextInfo::stopRequested()
    {
        if (!mRunning.test_and_set()) {
            Closure<void(ContinuationMode)> callback = std::move(mContext->mCallback);
            callback(ContinuationMode::Abort);
        }
    }

}
}
