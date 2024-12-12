#include "../../moduleslib.h"

#include "awaitabletimepoint.h"

#include "../taskhandle.h"
#include "../taskqueue.h"

Engine::Threading::TaskQualifiers operator co_await(std::chrono::steady_clock::time_point timepoint)
{
    return timepoint;
}

Engine::Threading::TaskQualifiers operator co_await(std::chrono::steady_clock::duration dur)
{
    return dur;
}
