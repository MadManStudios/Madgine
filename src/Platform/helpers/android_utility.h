#pragma once

#if ANDROID

namespace Engine {
namespace Platform {
    namespace Android {

        PLATFORM_EXPORT void triggerRumble(std::chrono::milliseconds duration);

    }
}
}

#endif