#include "../debuglib.h"

#include "debugger.h"

#include "Modules/uniquecomponent/uniquecomponentcollector.h"

#include "Meta/keyvalue/metatable_impl.h"

#include "debuglistener.h"

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

    void ContextInfo::suspend(Continuation callback, std::stop_token st)
    {
        if (mStopRequested) {
            callback(ContinuationMode::Abort);
            return;
        }

        for (DebugListener *listener : Debugger::getSingleton().mListeners)
            listener->onSuspend(*this, callback.type());
        mCallback = std::move(callback);
        int initialState = 0;
        mPaused.compare_exchange_strong(initialState, 1);
        assert(initialState == 0);
        mStopCallback.start(std::move(st), stop_cb { this });
        initialState = 1;
        mPaused.compare_exchange_strong(initialState, 2);
    }

    void ContextInfo::continueExecution(ContinuationMode mode)
    {
        int initialState = 2;
        if (mPaused.compare_exchange_strong(initialState, 3)) {
            mStopCallback.finish(mode);
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
        return mPaused == 2;
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

    bool ContextInfo::stop_cb::operator()() const
    {
        int initialState = 1;
        if (mContext->mPaused.compare_exchange_strong(initialState, 3))
            return true;
        initialState = 2;
        return mContext->mPaused.compare_exchange_strong(initialState, 3);
    }

    void ContextInfo::finally_cb::operator()(ContinuationMode mode) const
    {
        assert(mContext->mPaused == 3);
        Closure<void(ContinuationMode)> callback = std::move(mContext->mCallback);
        mContext->mPaused = 0;
        callback(mode);
    }

    void ContextInfo::finally_cb::operator()(Execution::cancelled_t) const
    {
        operator()(ContinuationMode::Abort);
    }

}
}
