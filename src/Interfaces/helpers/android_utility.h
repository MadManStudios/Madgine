#pragma once

#if ANDROID

namespace Engine {
namespace Android {

    INTERFACES_EXPORT void triggerRumble(std::chrono::milliseconds duration);

}
}

#endif