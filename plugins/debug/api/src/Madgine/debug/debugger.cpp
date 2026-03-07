#include "../debuglib.h"

#include "debugger.h"

#include "Modules/uniquecomponent/uniquecomponentcollector.h"

#include "Meta/keyvalue/metatable_impl.h"

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

}
}
