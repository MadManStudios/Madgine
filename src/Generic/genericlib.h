#pragma once

/// @cond

#include "commonlib.h"

#include <algorithm>
#include <array>
#include <assert.h>
#include <chrono>
#include <compare>
#include <deque>
#include <functional>
#include <future>
#include <istream>
#include <list>
#include <map>
#include <set>
#include <sstream>
#include <stdint.h>
#include <string>
#include <utility>
#include <variant>
#include <vector>
#include <typeindex>

#include "genericconfig.h"
#include "genericforward.h"

// clang-format off
#include "concepts.h"

#include "templates.h"

#include "auto_pack.h"
#include "type_pack.h"
#include "tag_invoke.h"

#include "derive.h"

#include "compatibility/compatibility.h"


#include "callable_traits.h"
#include "stringutil.h"

#include "container/container_traits.h"
#include "tupleunpacker.h"
#include "typeunpacker.h"
// clang-format on

using namespace std::literals::chrono_literals;
using namespace std::literals::string_literals;

/// @endcond
