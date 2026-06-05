#pragma once

#include "Generic/containers/virtualrange.h"

namespace Engine {
namespace Behavior {
    namespace Python3 {

        extern PyTypeObject PyVirtualSequenceIteratorType;

        struct PyVirtualSequenceIterator {
            PyObject_HEAD
                Reflect::SequenceIterator mIt;
        };

        extern PyTypeObject PyVirtualAssociativeIteratorType;

        struct PyVirtualAssociativeIterator {
            PyObject_HEAD
                Reflect::AssociativeIterator mIt;
        };

    }
}
}
