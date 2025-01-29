#pragma once

#include "Generic/execution/algorithm.h"
#include "Madgine/bindings.h"

#include "../widget.h"

namespace Engine {
namespace Widgets {

    using WidgetBinding = Binding<"Widget", WidgetBase*>;
    constexpr WidgetBinding widgetBinding;

}
}
