#pragma once

#include "../taskqueue.h"

MODULES_EXPORT Engine::Threading::TaskQualifiers operator co_await(std::chrono::steady_clock::time_point point);
MODULES_EXPORT Engine::Threading::TaskQualifiers operator co_await(std::chrono::steady_clock::duration dur);
