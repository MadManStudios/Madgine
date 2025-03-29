#pragma once

#include "Generic/container/virtualrange.h"

namespace Engine {
namespace Scripting {
    namespace Python3 {

        extern PyTypeObject PyVirtualSequenceIteratorType;

        struct PyVirtualSequenceIterator {
            PyObject_HEAD
                VirtualIterator<ValueType>
                    mIt;
        };

        extern PyTypeObject PyVirtualAssociativeIteratorType;

        struct PyVirtualAssociativeIterator {
            PyObject_HEAD
                VirtualIterator<KeyValuePair>
                    mIt;
        };

    }
}
}
