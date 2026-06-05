#pragma once

namespace Engine {
namespace Platform {
    namespace Log {

        struct PLATFORM_EXPORT Log {
            virtual ~Log() = default;
            virtual void log(std::string_view msg, MessageType lvl, const char *file = nullptr, size_t line = 0);
            virtual std::string getName() = 0;

            static void addListener(LogListener *listener);
            static void removeListener(LogListener *listener);
        };

    }
}
}
