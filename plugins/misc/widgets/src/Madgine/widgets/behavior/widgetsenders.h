#pragma once

#include "Generic/execution/algorithm.h"
#include "Madgine/bindings.h"
#include "Generic/intervalclock.h"

#include "../widget.h"

#include "Madgine/nativebehaviorcollector.h"

#include "../widgetmanager.h"

namespace Engine {
namespace Widgets {

    using WidgetBinding = Binding<"Widget", WidgetBase *>;
    constexpr WidgetBinding widgetBinding;

    using WidgetManagerBinding = Binding<"WidgetManager", WidgetManager *>;
    constexpr WidgetManagerBinding widgetManagerBinding;

    constexpr auto wait_frame = [](std::chrono::steady_clock::duration duration, WidgetManagerBinding manager = {}) {
        return IntervalClock<>::wait(manager | Execution::then(&WidgetManager::clock), duration);
    };

    constexpr auto yield_frame = [](WidgetManagerBinding manager = {}) {
        return wait_frame(0s, manager);
    };

}
}

NATIVE_BEHAVIOR_DECLARATION(Yield_Frame)
NATIVE_BEHAVIOR_DECLARATION(Wait_Frame)
