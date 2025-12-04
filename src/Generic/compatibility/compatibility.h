#pragma once

#ifdef __has_include
#    if __has_include(<version>)
#        include <version>
#    endif
#endif

#ifdef COMPATIBILITY_CONFIG_HEADER
#    include COMPATIBILITY_CONFIG_HEADER
#endif

#include "atomic.h"
#include "bit.h"
#include "concepts.h"
#include "coroutine.h"
#include "memory_resource.h"
#include "ranges.h"
#include "spaceship.h"

#if __cpp_lib_constexpr_algorithms >= 201806L
#    define CONSTEXPR_ALGORITHM constexpr
#else
#    define CONSTEXPR_ALGORITHM inline
#endif

#if __cpp_consteval >= 201811L
#    define CONSTEVAL consteval
#else
#    define CONSTEVAL constexpr
#endif