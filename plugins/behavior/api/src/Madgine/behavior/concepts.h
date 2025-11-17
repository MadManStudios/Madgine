#pragma once


#include "Generic/execution/concepts.h"


namespace Engine {
namespace Behavior {

    template <typename T>
    concept UntypedBehavior = Execution::Sender<T>;

    template <typename T, typename R>
    concept TypedBehavior = UntypedBehavior<T>;

}
}