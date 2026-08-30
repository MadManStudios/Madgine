#pragma once

#include "Generic/execution/algorithm.h"
#include "Generic/execution/binding.h"
#include "Generic/execution/intervalclock.h"

#include "Madgine/behavior/context.h"

#include "../widget.h"
#include "../widgetmanager.h"

namespace Engine {
namespace Widgets {

    using WidgetContext = Behavior::ContextParameter<Execution::BindingPtr<WidgetBase &>>;
    using WidgetManagerContext = Behavior::ContextParameter<WidgetManager &>;

    constexpr auto wait_frame = [](std::chrono::steady_clock::duration duration, WidgetManagerContext manager = {}, std::chrono::steady_clock::duration durationOverride = -1s, std::chrono::steady_clock::duration acc = 0s) {
        return std::move(manager) | Execution::let_value([=](auto &&manager) { return manager.clock().wait(duration, durationOverride, acc); });
    };

    constexpr auto yield_frame = [](WidgetManagerContext manager = {}, std::chrono::steady_clock::duration duration = 0s, std::chrono::steady_clock::duration acc = 0s) {
        return wait_frame(0s, manager, duration, acc);
    };

    MADGINE_WIDGETS_EXPORT Behavior::Behavior animate_move(Math::Matrix3 dist, std::chrono::nanoseconds duration, WidgetContext widget = {});
    MADGINE_WIDGETS_EXPORT Behavior::Behavior animate_opacity(float dist, std::chrono::nanoseconds duration, WidgetContext widget = {});

}
}
