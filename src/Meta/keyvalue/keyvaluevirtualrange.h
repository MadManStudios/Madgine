#pragma once

#include "Generic/container/virtualrange.h"

namespace Engine {

using KeyValueVirtualAssociativeIterator = VirtualIterator<KeyValuePair>;
using KeyValueVirtualAssociativeRange = VirtualRange<KeyValuePair, Functor_to_KeyValuePair>;
using KeyValueVirtualSequenceIterator = VirtualIterator<ValueType>;
using KeyValueVirtualSequenceRange = VirtualRange<ValueType, Functor_to_ValueType>;


}