#pragma once

#include "Generic/execution/algorithm.h"
#include "Generic/execution/binding.h"
#include "Generic/intervalclock.h"
#include "Madgine/named.h"

#include "../widget.h"

#include "Madgine/nativebehaviorcollector.h"

#include "../widgetmanager.h"

namespace Engine {
namespace Widgets {

    using WidgetBinding = Named<"Widget", Execution::BindingPtr<WidgetBase *>>;
    constexpr WidgetBinding widgetBinding;

    using WidgetManagerBinding = Named<"WidgetManager", Execution::ConstantBinding<WidgetManager *>>;
    constexpr WidgetManagerBinding widgetManagerBinding;

    constexpr auto wait_frame = []<typename Binding = const WidgetManagerBinding &>(std::chrono::steady_clock::duration duration, Binding &&manager = widgetManagerBinding) {
        return std::forward<Binding>(manager) | Execution::let_value([=](auto &&manager) { return IntervalClock<>::wait((std::forward<decltype(manager)>(manager)->*&WidgetManager::clock)(), duration); });
    };

    constexpr auto yield_frame = []<typename Binding = const WidgetManagerBinding &>(Binding &&manager = widgetManagerBinding) {
        return wait_frame(0s, manager);
    };

    MADGINE_WIDGETS_EXPORT Behavior animate_move(Matrix3 dist, std::chrono::nanoseconds duration, WidgetBinding widget = {});
    MADGINE_WIDGETS_EXPORT Behavior animate_opacity(float dist, std::chrono::nanoseconds duration, WidgetBinding widget = {});

}
}
