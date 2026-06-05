#pragma once

namespace Engine {
namespace Platform {
    namespace Threading {

        PLATFORM_EXPORT void setCurrentThreadName(const std::string &name);
        PLATFORM_EXPORT std::string getCurrentThreadName();

    }
}
}
