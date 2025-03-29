#pragma once

#include "../debug/stacktrace.h"

namespace Engine {
namespace Log {

    struct LogListener {
        virtual ~LogListener() = default;
        virtual void messageLogged(std::string_view message, MessageType lml, const char *file, size_t line, Log *log) = 0;
    };

}
}
