#pragma once

#include "Meta/flagsholder.h"

namespace Engine {
namespace Behavior {
    namespace Python3 {

        extern PyTypeObject PyFlagsType;

        struct PyFlags {
            PyObject_HEAD
                FlagsHolder mFlags;
        };

    }
}
}
