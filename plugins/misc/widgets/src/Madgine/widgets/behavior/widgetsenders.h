#pragma once

#include "Generic/execution/algorithm.h"
#include "Generic/execution/binding.h"
#include "Generic/intervalclock.h"
#include "Madgine/behavior/named.h"

#include "../widget.h"

#include "../widgetmanager.h"

namespace Engine {
namespace Widgets {

    using WidgetBinding = Behavior::Named<"Widget", Execution::BindingPtr<WidgetBase &>>;    
    using NamedWidgetManager = Behavior::Named<"WidgetManager", WidgetManager &>;

    constexpr auto wait_frame = [](std::chrono::steady_clock::duration duration, NamedWidgetManager manager = {}, std::chrono::steady_clock::duration durationOverride = -1s, std::chrono::steady_clock::duration acc = 0s) {
        return manager | Execution::let_value([=](auto &&manager) { return manager.clock().wait(duration, durationOverride, acc); });
    };

    constexpr auto yield_frame = [](NamedWidgetManager manager = {}, std::chrono::steady_clock::duration duration = 0s, std::chrono::steady_clock::duration acc = 0s) {
        return wait_frame(0s, manager, duration, acc);
    };

    MADGINE_WIDGETS_EXPORT Behavior::Behavior animate_move(Matrix3 dist, std::chrono::nanoseconds duration, WidgetBinding widget = {});
    MADGINE_WIDGETS_EXPORT Behavior::Behavior animate_opacity(float dist, std::chrono::nanoseconds duration, WidgetBinding widget = {});

}
}
