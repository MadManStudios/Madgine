#include "../interfaceslib.h"

#include "log.h"
#include "loglistener.h"

namespace Engine {
namespace Log {

    std::list<LogListener *> sListeners;

    void Log::log(std::string_view msg, MessageType lvl, const char *file, size_t line)
    {
        for (LogListener *listener : sListeners) {
            listener->messageLogged(msg, lvl, file, line, this);
        }
    }

    void Log::addListener(LogListener *listener)
    {
        sListeners.push_back(listener);
    }

    void Log::removeListener(LogListener *listener)
    {
        sListeners.remove(listener);
    }

}
}