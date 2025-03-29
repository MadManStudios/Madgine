#pragma once

#include "Generic/container/virtualrange.h"

namespace Engine {
namespace Scripting {
    namespace Python3 {

        extern PyTypeObject PyVirtualAssociativeRangeType;

        struct PyVirtualAssociativeRange {
            PyObject_HEAD
                KeyValueVirtualAssociativeRange mRange;
        };

        
        extern PyTypeObject PyVirtualSequenceRangeType;

        struct PyVirtualSequenceRange {
            PyObject_HEAD
                KeyValueVirtualSequenceRange mRange;
        };


    }
}
}
