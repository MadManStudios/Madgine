#pragma once

#include "Generic/containers/virtualrange.h"

namespace Engine {
namespace Reflect {

    using AssociativeIterator = Containers::VirtualIterator<const Value &, const Value &>;
    using AssociativeRange = Containers::VirtualRange<VirtualRangeHelper, const Value &, const Value &>;
    using SequenceIterator = Containers::VirtualIterator<const Value &>;
    using SequenceRange = Containers::VirtualRange<VirtualRangeHelper, const Value &>;

}
}