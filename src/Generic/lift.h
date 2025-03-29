#pragma once

#define LIFT(fname, ...)                                       \
    [__VA_ARGS__](auto &&...args) -> decltype(auto) {          \
        return (fname)(std::forward<decltype(args)>(args)...); \
    }
