#pragma once

#ifndef MADGINE_COMPATIBILITY_NEED_STOP_TOKEN
#    define MADGINE_COMPATIBILITY_NEED_STOP_TOKEN (__cpp_lib_jthread < 201911L)
#endif

#if !MADGINE_COMPATIBILITY_NEED_STOP_TOKEN

#include <stop_token>

#else

namespace std {
#    if __cpp_lib_jthread > 0L
#        pragma message "Using fallback for std::stop_token. (__cpp_lib_jthread: " STRINGIFY2(__cpp_lib_jthread) ")"
#    else
#        pragma message "Using fallback for std::stop_token. (__cpp_lib_jthread: undefined)"
#    endif

struct stop_token {
    bool stop_requested()
    {
        return false;
    }
};
template <typename F>
struct stop_callback {
    template <typename... T>
    stop_callback(T &&...) { }
};

constexpr struct nostopstate_t {
} nostopstate;
struct stop_source {
    stop_source() {};
    stop_source(nostopstate_t) {};
    stop_token get_token()
    {
        return {};
    }
    bool request_stop()
    {
        return true;
    }
    bool stop_possible()
    {
        return true;
    }
    bool stop_requested()
    {
        return false;
    }
};
}
#endif
