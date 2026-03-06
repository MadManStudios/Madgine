#pragma once

#include "Generic/execution/concepts.h"

namespace Engine {
namespace Behavior {

    template <typename T>
    concept UntypedBehavior = Execution::AnySender<T>;

    template <typename T, typename R>
    concept TypedBehavior = UntypedBehavior<T>;

}
}