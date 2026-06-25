#include "../debuglib.h"

#include "contextinfo.h"

#include "Generic/execution/stop_source.h"

#include "Meta/reflect/metatable_impl.h"

#include "debugger.h"
#include "debuglistener.h"

METATABLE_BEGIN(Engine::Debug::ContextInfo)
    READONLY_PROPERTY(Alive, alive)
METATABLE_END(Engine::Debug::ContextInfo)

namespace Engine {
namespace Debug {

    bool ContextInfo::wantsPause(TypedPtr location, ContinuationType type, IndexType<size_t> line)
    {
        bool pause = (line && (mPauseRequested || type == Debug::ContinuationType::Error || getBreakpoint(location.ptr(), line)))
            || mStopRequested;

        for (DebugListener *listener : Debugger::getSingleton().mListeners) {
            pause |= listener->wantsPause(*this, location, type, line);
        }

        return pause;
    }

    Continuation ContextInfo::suspend(TypedPtr location, Continuation callback)
    {
        for (DebugListener *listener : Debugger::getSingleton().mListeners)
            listener->onSuspend(*this, location, callback.type());

        return callback;
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
        return static_cast<bool>(mChild);
    }

    bool ContextInfo::isPaused() const
    {
        return false;
    }

    void ContextInfo::setBreakpoint(const void *location, size_t index, bool set) const
    {
        auto &data = mDebugLocations[location];

        if (data.mBreakpoints.size() <= index) {
            data.mBreakpoints.resize(index + 1);
        }
        data.mBreakpoints[index] = set;
    }

    bool ContextInfo::getBreakpoint(const void *location, size_t index) const
    {
        auto it = mDebugLocations.find(location);
        if (it == mDebugLocations.end())
            return false;
        return it->second.mBreakpoints.size() > index && it->second.mBreakpoints[index];
    }

}
}