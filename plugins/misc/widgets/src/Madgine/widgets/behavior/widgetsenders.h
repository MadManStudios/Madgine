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

    struct WidgetBinding : Named<"Widget", Execution::BindingPtr<WidgetBase *>> {
        template <typename F>
        decltype(auto) sender(F &&f)
        {
            return Named<"Widget", Execution::BindingPtr<WidgetBase *>>::sender([&](auto binding) {
                return (binding->*std::forward<F>(f))();
            });
        }
    };
    using NamedWidgetManager = Named<"WidgetManager", WidgetManager *>;

    constexpr auto wait_frame = [](std::chrono::steady_clock::duration duration, NamedWidgetManager manager = {}) {
        return manager.sender([=](auto &&manager) { return manager->clock().wait(duration) | Execution::with_debug_location<Debug::SenderLocation>(); });
    };

    constexpr auto yield_frame = [](NamedWidgetManager manager = {}) {
        return wait_frame(0s, manager);
    };

    MADGINE_WIDGETS_EXPORT Behavior animate_move(Matrix3 dist, std::chrono::nanoseconds duration, WidgetBinding widget = {});
    MADGINE_WIDGETS_EXPORT Behavior animate_opacity(float dist, std::chrono::nanoseconds duration, WidgetBinding widget = {});

}
}
