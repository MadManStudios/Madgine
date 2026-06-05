#pragma once

#if defined(__clang__)
#    define MADGINE_BREAKPOINT() __builtin_debugtrap()
#elif defined(_MSC_VER)
#    define MADGINE_BREAKPOINT() __debugbreak()
#else
#    define MADGINE_BREAKPOINT() assert(false)
#endif