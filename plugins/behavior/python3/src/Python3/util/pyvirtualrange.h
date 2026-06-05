#pragma once

#include "Generic/containers/virtualrange.h"

namespace Engine {
namespace Behavior {
    namespace Python3 {

        extern PyTypeObject PyVirtualAssociativeRangeType;

        struct PyVirtualAssociativeRange {
            PyObject_HEAD
                Reflect::AssociativeRange mRange;
        };

        extern PyTypeObject PyVirtualSequenceRangeType;

        struct PyVirtualSequenceRange {
            PyObject_HEAD
                Reflect::SequenceRange mRange;
        };

    }
}
}
