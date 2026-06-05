#pragma once

#include "Generic/containers/virtualrange.h"

namespace Engine {
namespace Reflect {

    using AssociativeIterator = Containers::VirtualIterator<KeyValuePair>;
    using AssociativeRange = Containers::VirtualRange<KeyValuePair, Functor_toKeyValuePair>;
    using SequenceIterator = Containers::VirtualIterator<Value>;
    using SequenceRange = Containers::VirtualRange<Value, Functor_toValue>;

}
}